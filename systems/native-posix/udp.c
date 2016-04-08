#include "freertps/freertps.h"
#include "freertps/timer.h"
#include "freertps/udp.h"
#include "freertps/disco.h"
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

// tragedy this didn't get into POSIX...
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#define FRUDP_MAX_RX_SOCKS 10

static freertps_timer_cb_t g_timer_cb = NULL;
static double g_timer_period = -1;
static double g_timer_last_t = 0;

typedef struct
{
  int sock;
  uint16_t port;
  uint32_t addr;
  //freertps_udp_rx_callback_t cb;
} frudp_rx_sock_t;

static frudp_rx_sock_t g_frudp_rx_socks[FRUDP_MAX_RX_SOCKS];
static int g_frudp_rx_socks_used;

//static struct in_addr g_frudp_tx_addr;
static struct sockaddr_in g_frudp_tx_addr;
static int g_frudp_tx_sock;

#define FU_RX_BUFSIZE 4096

bool frudp_init(void)
{
  FREERTPS_INFO("udp init()\n");
  for (int i = 0; i < FRUDP_MAX_RX_SOCKS; i++)
  {
    g_frudp_rx_socks[i].sock = -1;
    //g_freertps_udp_rx_socks[i].cb = NULL;
  }

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
    FREERTPS_INFO("found address %s on interface %s\n", host, ifa->ifa_name);
    if (0 == strcmp(host, "127.0.0.1"))
      continue; // boring
    tx_addr_str = host; // save this one for now
  }
  FREERTPS_INFO("using address %s for unicast\n", tx_addr_str);
  g_frudp_tx_addr.sin_addr.s_addr = inet_addr(tx_addr_str);
  g_frudp_config.unicast_addr = (uint32_t)g_frudp_tx_addr.sin_addr.s_addr;
  freeifaddrs(ifaddr);

  g_frudp_tx_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (g_frudp_tx_sock < 0)
  {
    FREERTPS_FATAL("couldn't create tx sock\n");
    return false;
  }
  int result;
  result = setsockopt(g_frudp_tx_sock, IPPROTO_IP, IP_MULTICAST_IF,
                      (char *)&g_frudp_tx_addr.sin_addr.s_addr,
                      sizeof(g_frudp_tx_addr));
  if (result < 0)
  {
    FREERTPS_FATAL("couldn't set tx sock to allow multicast\n");
    return false;
  }

  // because we may have multiple freertps processes listening to the
  // multicast traffic on this host, we need to enable multicast loopback
  // todo: I assume this is enabled by default, but let's set it anyway
  int loopback = 1;
  result = setsockopt(g_frudp_tx_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                      &loopback, sizeof(loopback));
  if (result < 0)
  {
    FREERTPS_FATAL("couldn't enable outbound tx multicast loopback\n");
    return false;
  }
  
  //if (!frudp_init_participant_id())
  //  return false;
  // some of the following stuff has been moved to frudp_part_create()
  //frudp_generic_init();
  // not sure about endianness here.
  g_frudp_config.guid_prefix.prefix[0] = FREERTPS_VENDOR_ID >> 8;
  g_frudp_config.guid_prefix.prefix[1] = FREERTPS_VENDOR_ID & 0xff;
  // todo: actually get mac address
  const uint8_t mac[6] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab };
  memcpy(&g_frudp_config.guid_prefix.prefix[2], mac, 6);
  // 4 bytes left. let's use the POSIX process ID
  uint32_t pid = (uint32_t)getpid(); // on linux, this will be 4 bytes
  memcpy(&g_frudp_config.guid_prefix.prefix[8], &pid, 4);
  //frudp_disco_init();
  frudp_generic_init();
  return true;
}

bool frudp_init_participant_id(void)
{
  FREERTPS_INFO("frudp_init_participant_id()\n");
  for (int pid = 0; pid < 100; pid++) // todo: hard upper bound is bad
  {
    // see if we can open the port; if so, let's say we have a unique PID
    g_frudp_config.participant_id = pid;
    const uint16_t port = frudp_ucast_builtin_port();

    if (frudp_add_ucast_rx(port))
    {
      FREERTPS_INFO("using RTPS/DDS PID %d\n", pid);
      return true;
    }
  }
  return false; // couldn't find an available PID
}

void frudp_fini(void)
{
  frudp_disco_fini();
  FREERTPS_INFO("udp fini\n");
  for (int i = 0; i < g_frudp_rx_socks_used; i++)
  {
    close(g_frudp_rx_socks[i].sock);
    g_frudp_rx_socks[i].sock = -1;
  }
}

static int frudp_create_sock(void)
{
  if (g_frudp_rx_socks_used >= FRUDP_MAX_RX_SOCKS)
  {
    FREERTPS_ERROR("oh noes, i ran out of socks");
    return -1;
  }
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    FREERTPS_ERROR("couldn't create socket");
  return s;
}

bool frudp_add_ucast_rx(const uint16_t port)
{
  FREERTPS_INFO("add ucast rx port %d\n", port);
  // we may have already added this when searching for our participant ID
  // so, let's spin through and see if it's already there
  for (int i = 0; i < g_frudp_rx_socks_used; i++)
    if (g_frudp_rx_socks[i].port == port)
    {
      FREERTPS_INFO("  found port match in slot %d\n", i);
      return true; // it's already here
    }
  int s = frudp_create_sock();
  if (s < 0)
    return false;
  struct sockaddr_in rx_bind_addr;
  memset(&rx_bind_addr, 0, sizeof(rx_bind_addr));
  rx_bind_addr.sin_family = AF_INET;
  rx_bind_addr.sin_addr.s_addr = g_frudp_tx_addr.sin_addr.s_addr;
  rx_bind_addr.sin_port = htons(port);
  int result = bind(s, (struct sockaddr *)&rx_bind_addr, sizeof(rx_bind_addr));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't bind to unicast port %d\n", port);
    close(s);
    return false;
  }
  frudp_rx_sock_t *rxs = &g_frudp_rx_socks[g_frudp_rx_socks_used];
  rxs->sock = s;
  rxs->port = port;
  rxs->addr = rx_bind_addr.sin_addr.s_addr;
  //FREERTPS_INFO("  added in rx sock slot %d\n", g_frudp_rx_socks_used);
  g_frudp_rx_socks_used++;
  return true;
}

static bool set_sock_reuse(int s)
{
  int one = 1;

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

bool frudp_add_mcast_rx(in_addr_t group, uint16_t port) //,
                     //const freertps_udp_rx_callback_t rx_cb)
{
  FREERTPS_INFO("add mcast rx port %d\n", port);
  int s = frudp_create_sock();
  if (s < 0)
    return false;

  if (!set_sock_reuse(s))
    return false;

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
    return false;
  }

  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = group;
  mreq.imr_interface.s_addr = g_frudp_tx_addr.sin_addr.s_addr;
  result = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't add rx sock to multicast group, errno = %d\n",
                   errno);
    return false;
  }
  frudp_rx_sock_t *rxs = &g_frudp_rx_socks[g_frudp_rx_socks_used];
  rxs->sock = s;
  rxs->port = port;
  rxs->addr = g_frudp_tx_addr.sin_addr.s_addr;
  //g_freertps_udp_rx_socks[g_freertps_udp_rx_socks_used].cb = rx_cb;
  //FREERTPS_INFO("  added in rx sock slot %d\n", g_frudp_rx_socks_used);
  g_frudp_rx_socks_used++;
  return true;
}

bool frudp_listen(const uint32_t max_usec)
{
  //printf("frudp_listen(%d)\n", (int)max_usec);
  static uint8_t s_frudp_listen_buf[FU_RX_BUFSIZE]; // haha
  double t_start = fr_time_now_double();
  double t_now = t_start;
  while (t_now - t_start <= 1.0e-6 * max_usec)
  {
    fd_set rdset;
    FD_ZERO(&rdset);
    int max_fd = 0;
    for (int i = 0; i < g_frudp_rx_socks_used; i++)
    {
      const int s = g_frudp_rx_socks[i].sock;
      FD_SET(s, &rdset);
      if (s > max_fd)
        max_fd = s;
    }
    double t_elapsed = t_now - t_start;
    double t_remaining = (max_usec * 1.0e-6) - t_elapsed;
    if (g_timer_period > 0)
    {
      double t_until_timeout = g_timer_last_t + g_timer_period - t_now;
      if (t_until_timeout < 0)
        t_remaining = 0;
      else if (t_until_timeout < t_remaining)
        t_remaining = t_until_timeout;
    }
    struct timeval timeout;
    timeout.tv_sec = floor(t_remaining);// max_usec / 1000000;
    timeout.tv_usec = (t_remaining - timeout.tv_sec) * 1000000;  /*max_usec - timeout.tv_sec * 1000000*/;
    int rv = select(max_fd+1, &rdset, NULL, NULL, &timeout);
    if (rv < 0)
      return false; // badness
    if (rv > 0)
    {
      // find out which of our rx socks had something exciting happen
      for (int i = 0; i < g_frudp_rx_socks_used; i++)
      {
        frudp_rx_sock_t *rxs = &g_frudp_rx_socks[i];
        if (FD_ISSET(rxs->sock, &rdset))
        {
          struct sockaddr_in src_addr;
          int addrlen = sizeof(src_addr);
          int nbytes = recvfrom(rxs->sock,
                                s_frudp_listen_buf, sizeof(s_frudp_listen_buf),
                                0,
                                (struct sockaddr *)&src_addr,
                                (socklen_t *)&addrlen);
          //printf("rx %d\r\n", nbytes);
          frudp_rx(src_addr.sin_addr.s_addr, src_addr.sin_port,
                   rxs->addr, rxs->port,
                   s_frudp_listen_buf, nbytes);
        }
      }
    }
    if (max_usec == 0) // this is a common case from, e.g., spin_some()
      break;
    t_now = fr_time_now_double();
    if (g_timer_period > 0)
    {
      double t_until_timeout = g_timer_last_t + g_timer_period - t_now;
      if (t_until_timeout <= 0)
      {
        g_timer_last_t = t_now;
        g_timer_cb();
      }
    }
  }
  return true;
}

bool frudp_tx(const in_addr_t dst_addr,
              const in_port_t dst_port,
              const uint8_t *tx_data,
              const uint16_t tx_len)
{
  //struct sockaddr_in g_freertps_tx_addr;
  g_frudp_tx_addr.sin_port = htons(dst_port);
  g_frudp_tx_addr.sin_addr.s_addr = dst_addr;
  // todo: be smarter
  if (tx_len == sendto(g_frudp_rx_socks[3].sock, tx_data, tx_len, 0,
                       (struct sockaddr *)(&g_frudp_tx_addr),
                       sizeof(g_frudp_tx_addr)))
    return true;
  else
    return false;
}

void freertps_timer_set_freq(uint32_t freq, freertps_timer_cb_t cb)
{
  g_timer_period = 1.0 / (double)freq;
  g_timer_cb = cb;
}
