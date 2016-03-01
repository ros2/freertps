#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <math.h>
#include "freertps/freertps.h"
#include "freertps/participant.h"
#include "freertps/rc.h"
#include "freertps/timer.h"
#include "freertps/udp.h"

// tragedy this didn't get into POSIX...
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

// todo: replace this with something smarter... though it's nice for this
// to be super efficient, too bad that won't scale to all situations... :(
#define FRUDP_MAX_RX_SOCKS 10
typedef struct
{
  int sock;
  uint16_t port;
  uint32_t addr;
  //freertps_udp_rx_callback_t cb;
} fr_rx_sock_t;

static fr_rx_sock_t g_fr_rx_socks[FRUDP_MAX_RX_SOCKS];
static int g_fr_rx_socks_used;

//static struct in_addr g_fr_tx_addr;
static struct sockaddr_in g_fr_tx_addr;
static int g_fr_tx_sock;

static uint32_t g_fr_system_unicast_addr;

static bool set_sock_reuse(int s);

#define FU_RX_BUFSIZE 4096

bool fr_system_udp_init()
{
  FREERTPS_INFO("fr_system_udp_init()\n");
  for (int i = 0; i < FRUDP_MAX_RX_SOCKS; i++)
    g_fr_rx_socks[i].sock = -1;

  struct ifaddrs *ifaddr;
  if (getifaddrs(&ifaddr) == -1)
  {
    FREERTPS_FATAL("couldn't call getifaddrs");
    return false;
  }
  char *tx_addr_str = "127.0.0.1"; // use loopback if nothing else is found
  for (struct ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next)
  {
    if (!ifa->ifa_addr)
      continue;
    int family = ifa->ifa_addr->sa_family;
    if (family != AF_INET)
      continue;
    char host[NI_MAXHOST];
    if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST))
      continue;
    FREERTPS_INFO("  found address %s on interface %s\n", host, ifa->ifa_name);
    if (0 == strcmp(host, "127.0.0.1"))
      continue; // boring
    tx_addr_str = host; // save this one for now
  }
  FREERTPS_INFO("  using address %s for unicast\n", tx_addr_str);
  g_fr_tx_addr.sin_addr.s_addr = inet_addr(tx_addr_str);
  g_fr_system_unicast_addr = (uint32_t)g_fr_tx_addr.sin_addr.s_addr;
  freeifaddrs(ifaddr);

  g_fr_tx_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (g_fr_tx_sock < 0)
  {
    FREERTPS_FATAL("couldn't create tx sock\n");
    return false;
  }
  int result;
  result = setsockopt(g_fr_tx_sock, IPPROTO_IP, IP_MULTICAST_IF,
                      (char *)&g_fr_tx_addr.sin_addr.s_addr,
                      sizeof(g_fr_tx_addr));
  if (result < 0)
  {
    FREERTPS_FATAL("couldn't set tx sock to allow multicast\n");
    return false;
  }

  // because we may have multiple freertps processes listening to the
  // multicast traffic on this host, we need to enable multicast loopback
  // todo: I assume this is enabled by default, but let's set it anyway
  int loopback = 1;
  result = setsockopt(g_fr_tx_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                      &loopback, sizeof(loopback));
  if (result < 0)
  {
    FREERTPS_FATAL("couldn't enable outbound tx multicast loopback\n");
    return false;
  }

 
  //if (!fr_init_participant_id())
  //  return false;
  // some of the following stuff has been moved to fr_part_create()
  // not sure about endianness here.
  g_fr_participant.guid_prefix.prefix[0] = FREERTPS_VENDOR_ID >> 8;
  g_fr_participant.guid_prefix.prefix[1] = FREERTPS_VENDOR_ID & 0xff;
  // todo: actually get mac address
  const uint8_t mac[6] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab };
  memcpy(&g_fr_participant.guid_prefix.prefix[2], mac, 6);
  // 4 bytes left. let's use the POSIX process ID
  uint32_t pid = freertps_htonl((uint32_t)getpid()); // on linux, this will be 4 bytes
  memcpy(&g_fr_participant.guid_prefix.prefix[8], &pid, 4);

  int id = 0;
  const int MAX_PID = 100; // todo: hard upper bound is bad
  for (; id < MAX_PID; id++) 
  {
    // see if we can open the port; if so, let's say we have a unique PID
    g_fr_participant.participant_id = id;
    const uint16_t port = fr_participant_ucast_builtin_port();
    if (fr_add_ucast_rx(
        0, port, g_fr_participant.builtin_unicast_locators) == FR_RC_OK)
    {
      FREERTPS_INFO("  using RTPS/DDS PID %d\n", id);
      break;
    }
  }
  if (id >= MAX_PID)
  {
    printf("couldn't find an available user port after going through PID=%d\n",
        (int)MAX_PID);
    return false;
  }

  // for RTI compatibility, we need to always listen on the loopback address,
  // but we shouldn't add that to our list of locators to announce
  if (FR_RC_OK !=
      fr_add_ucast_rx(0x0100007f, fr_participant_ucast_builtin_port(), NULL))
  {
    FREERTPS_FATAL("couldn't listen on localhost!\n");
    return false;
  }

  return true;
}

void fr_system_udp_fini()
{
  fr_participant_discovery_fini();
  FREERTPS_INFO("fr_system_udp_fini\n");
  for (int i = 0; i < g_fr_rx_socks_used; i++)
  {
    close(g_fr_rx_socks[i].sock);
    g_fr_rx_socks[i].sock = -1;
  }
}

static int fr_create_sock()
{
  if (g_fr_rx_socks_used >= FRUDP_MAX_RX_SOCKS)
  {
    FREERTPS_ERROR("oh noes, i ran out of socks");
    return -1;
  }
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    FREERTPS_ERROR("couldn't create socket");
  return s;
}

fr_rc_t fr_add_ucast_rx
    (const uint32_t addr, const uint16_t port, struct fr_container *c)
{
  // we may have already added this when searching for our participant ID
  // so, let's spin through and see if it's already there
  //printf("fr_add_ucast_rx(addr=%d, port=%d)\n", (int)addr, (int)port);
  for (int i = 0; i < g_fr_rx_socks_used; i++)
    if (g_fr_rx_socks[i].port == port &&
        (!addr || g_fr_rx_socks[i].addr == addr))
    {
      FREERTPS_INFO("  found port match (%d) in slot %d\n", (int)port, i);
      return FR_RC_OK; // it's already here
    }
  int s = fr_create_sock();
  if (s < 0)
    return FR_RC_NETWORK_ERROR;
  // special-case loopback (for RTI compatibility)
  if (addr == 0x0100007f)
  {
    printf("setting SO_REUSEPORT on loopback port %d...\n", (int)port);
    if (!set_sock_reuse(s))
      return FR_RC_NETWORK_ERROR;
  }

  const uint32_t autodetect_addr = addr ? addr : g_fr_tx_addr.sin_addr.s_addr;

  struct sockaddr_in rx_bind_addr;
  memset(&rx_bind_addr, 0, sizeof(rx_bind_addr));
  rx_bind_addr.sin_family = AF_INET;
  rx_bind_addr.sin_addr.s_addr = autodetect_addr;
  rx_bind_addr.sin_port = htons(port);

  printf("native_posix fr_add_ucast_rx(%d.%d.%d.%d:%d)\n",
      (int)(autodetect_addr      ) & 0xff,
      (int)(autodetect_addr >>  8) & 0xff,
      (int)(autodetect_addr >> 16) & 0xff,
      (int)(autodetect_addr >> 24) & 0xff,
      (int)port);

  int result = bind(s, (struct sockaddr *)&rx_bind_addr, sizeof(rx_bind_addr));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't bind to unicast port %d\n", port);
    close(s);
    return FR_RC_NETWORK_ERROR;
  }
  fr_rx_sock_t *rxs = &g_fr_rx_socks[g_fr_rx_socks_used];
  rxs->sock = s;
  rxs->port = port;
  rxs->addr = autodetect_addr;
  //FREERTPS_INFO("  added in rx sock slot %d\n", g_fr_rx_socks_used);
  g_fr_rx_socks_used++;

  if (c)
    return fr_locator_container_append(g_fr_system_unicast_addr, port, c);
  else
    return FR_RC_OK;
  /*
  // now, add this locator to the participant's locator list
  struct fr_locator loc;
  loc.kind = FR_LOCATOR_KIND_UDPV4;
  loc.port = port;
  memset(loc.addr.udp4.zeros, 0, 12);
  loc.addr.udp4.addr = g_fr_system_unicast_addr;
  return fr_container_append(g_fr_participant.default_unicast_locators,
      &loc, sizeof(struct fr_locator), FR_CFLAGS_NONE);
  */
}

static bool set_sock_reuse(int s)
{
  int one = 1;

  // it looks like these branches are identical. why?
#if defined(SO_REUSEPORT) && !defined(__linux__)
  if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) < 0)
  {
    FREERTPS_ERROR("couldn't set SO_REUSEPORT on an rx sock (%d, %s)\n",
                   errno, strerror(errno));
    return false;
  }
#else
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
  {
    FREERTPS_ERROR("couldn't set SO_REUSEADDR on an rx sock (%d, %s)\n",
                   errno, strerror(errno));
    return false;
  }
#endif

  return true;
}

fr_rc_t fr_add_mcast_rx(in_addr_t group, uint16_t port, struct fr_container *c)
{
  //FREERTPS_INFO("add mcast rx port %d\n", port);
  int s = fr_create_sock();
  if (s < 0)
    return FR_RC_NETWORK_ERROR;

  if (!set_sock_reuse(s))
    return FR_RC_NETWORK_ERROR;

  int result;
  struct sockaddr_in rx_bind_addr;
  memset(&rx_bind_addr, 0, sizeof(rx_bind_addr));
  rx_bind_addr.sin_family = AF_INET;
  rx_bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  rx_bind_addr.sin_port = htons(port);
  result = bind(s, (struct sockaddr *)&rx_bind_addr, sizeof(rx_bind_addr));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't bind rx sock to port %d, failed with errno %d\n",
                   port, errno);
    return FR_RC_NETWORK_ERROR;
  }

  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = group;
  mreq.imr_interface.s_addr = g_fr_tx_addr.sin_addr.s_addr;
  result = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't add rx sock to multicast group, errno = %d\n",
                   errno);
    return FR_RC_NETWORK_ERROR;
  }
  fr_rx_sock_t *rxs = &g_fr_rx_socks[g_fr_rx_socks_used];
  rxs->sock = s;
  rxs->port = port;
  rxs->addr = g_fr_tx_addr.sin_addr.s_addr;
  g_fr_rx_socks_used++;

  return fr_locator_container_append(group, port, c);
  /*
  // now, add this locator to the participant's locator list
  struct fr_locator loc;
  loc.kind = FR_LOCATOR_KIND_UDPV4;
  loc.port = port;
  memset(loc.addr.udp4.zeros, 0, 12);
  loc.addr.udp4.addr = group;
  return fr_container_append(g_fr_participant.default_multicast_locators,
      &loc, sizeof(struct fr_locator), FR_CFLAGS_NONE);
  */
}
/*
  struct fr_locator loopback_locator;
  fr_locator_set_udp4(&loopback_locator, 0x7e000001, );
  fr_add_ucast_rx(fr_participant_ucast_builtin_port(), loopback_locator);
  fr_add_ucast_rx(fr_participant_ucast_user_port(), loopback_locator);
*/

int fr_system_listen_at_most(uint32_t microseconds)
{
  //printf("fr_listen(%d)\n", (int)max_usec);
  static uint8_t s_fr_listen_buf[FU_RX_BUFSIZE]; // haha

  fd_set rdset;
  FD_ZERO(&rdset);
  int max_fd = 0;
  for (int i = 0; i < g_fr_rx_socks_used; i++)
  {
    const int s = g_fr_rx_socks[i].sock;
    FD_SET(s, &rdset);
    if (s > max_fd)
      max_fd = s;
  }
  struct timeval timeout;
  timeout.tv_sec = microseconds / 1000000;
  timeout.tv_usec = microseconds - (timeout.tv_sec * 1000000);
  int rv = select(max_fd+1, &rdset, NULL, NULL, &timeout);
  if (rv < 0)
    return rv; // badness
  if (rv > 0)
  {
    // find out which of our rx socks had something exciting happen
    for (int i = 0; i < g_fr_rx_socks_used; i++)
    {
      fr_rx_sock_t *rxs = &g_fr_rx_socks[i];
      if (FD_ISSET(rxs->sock, &rdset))
      {
        struct sockaddr_in src_addr;
        int addrlen = sizeof(src_addr);
        int nbytes = recvfrom(rxs->sock,
                              s_fr_listen_buf, sizeof(s_fr_listen_buf),
                              0,
                              (struct sockaddr *)&src_addr,
                              (socklen_t *)&addrlen);
        //printf("rx %d\r\n", nbytes);
        fr_udp_rx(src_addr.sin_addr.s_addr, src_addr.sin_port,
                  rxs->addr, rxs->port,
                  s_fr_listen_buf, nbytes);
      }
    }
  }
  return rv;
}

bool fr_udp_tx(const in_addr_t dst_addr,
               const in_port_t dst_port,
               const uint8_t *tx_data,
               const uint16_t tx_len)
{
  //struct sockaddr_in g_freertps_tx_addr;
  g_fr_tx_addr.sin_port = htons(dst_port);
  g_fr_tx_addr.sin_addr.s_addr = dst_addr;
  // todo: be smarter
  /*
  printf("native_posix TX %d bytes to %d.%d.%d.%d:%d\n",
      (int)tx_len,
      (int)(dst_addr      ) & 0xff,
      (int)(dst_addr >>  8) & 0xff,
      (int)(dst_addr >> 16) & 0xff,
      (int)(dst_addr >> 24) & 0xff,
      (int)dst_port);
  */
  if (tx_len == sendto(g_fr_rx_socks[3].sock, tx_data, tx_len, 0,
                       (struct sockaddr *)(&g_fr_tx_addr),
                       sizeof(g_fr_tx_addr)))
    return true;
  else
    return false;
}


