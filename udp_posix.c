#include "freertps/freertps.h"
#include "freertps/udp.h"
#include "freertps/discovery.h"
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

// tragedy this didn't get into POSIX...
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#define FRUDP_MAX_RX_SOCKS 10

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

#define FU_RX_BUFSIZE 1500

bool frudp_init()
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
  FREERTPS_INFO("using address %s\n", tx_addr_str);
  g_frudp_tx_addr.sin_addr.s_addr = inet_addr(tx_addr_str);
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
  int loopback = 0;
  result = setsockopt(g_frudp_tx_sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                      &loopback, sizeof(loopback));
  if (result < 0)
  {
    FREERTPS_FATAL("couldn't disable outbound tx multicast loopback\n");
    return false;
  }
  frudp_generic_init();
  if (!frudp_init_participant_id())
    return false; 
  frudp_discovery_init();
  return true;
}

bool frudp_init_participant_id()
{
  FREERTPS_INFO("frudp_init_participant_id()\n");
  for (int i = 0; i < 100; i++) // todo: hard upper bound is bad
  {
    // see if we can open the port; if so, let's say we have a unique PID
    const uint16_t port = 7401 + 2 * i;
    if (frudp_add_ucast_rx(port))
    {
      // TODO: set global variable for pid
      break;
    }
  }
  return true;
}

void frudp_fini()
{
  frudp_discovery_fini();
  FREERTPS_INFO("udp fini\n");
  for (int i = 0; i < g_frudp_rx_socks_used; i++)
  {
    close(g_frudp_rx_socks[i].sock);
    g_frudp_rx_socks[i].sock = -1;
  }
}

static int frudp_create_sock()
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
  int s = frudp_create_sock();
  if (s < 0)
    return false;
  struct sockaddr_in rx_bind_addr;
  memset(&rx_bind_addr, 0, sizeof(rx_bind_addr));
  rx_bind_addr.sin_family = AF_INET;
  rx_bind_addr.sin_addr.s_addr = inet_addr("172.23.2.160"); // TODO: not this
  rx_bind_addr.sin_port = htons(port);
  int result = bind(s, (struct sockaddr *)&rx_bind_addr, sizeof(rx_bind_addr));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't bind to unicast port %d", port);
    close(s);
    return false;
  }
  frudp_rx_sock_t *rxs = &g_frudp_rx_socks[g_frudp_rx_socks_used];
  rxs->sock = s;
  rxs->port = port;
  rxs->addr = rx_bind_addr.sin_addr.s_addr;
  g_frudp_rx_socks_used++;
  return true;
}

bool frudp_add_mcast_rx(in_addr_t group, uint16_t port) //, 
                     //const freertps_udp_rx_callback_t rx_cb)
{
  FREERTPS_INFO("add mcast rx port %d\n", port);
  int s = frudp_create_sock();
  if (s < 0)
    return false;
  int result = 0, reuseaddr = 1;
  result = setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                      &reuseaddr, sizeof(reuseaddr));
  if (result < 0)
  {
    FREERTPS_ERROR("couldn't set SO_REUSEADDR on an rx sock\n");
    return false;
  }
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
  g_frudp_rx_socks_used++;
  return true;
}

bool frudp_listen(const uint32_t max_usec)
{
  static uint8_t s_frudp_listen_buf[FU_RX_BUFSIZE]; // haha 
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
  struct timeval timeout;
  timeout.tv_sec = max_usec / 1000000;
  timeout.tv_usec = max_usec - timeout.tv_sec * 1000000;
  int rv = select(max_fd+1, &rdset, NULL, NULL, &timeout);
  if (rv < 0)
    return false; // badness
  else if (rv == 0)
    return true;  // nothing to do, boring
  // now, find out which of our rx socks had something exciting happen
  for (int i = 0; i < g_frudp_rx_socks_used; i++)
  {
    frudp_rx_sock_t *rxs = &g_frudp_rx_socks[i];
    if (FD_ISSET(rxs->sock, &rdset))
    {
      struct sockaddr_in src_addr;
      int addrlen = sizeof(src_addr);
      int nbytes = recvfrom(rxs->sock,
                            s_frudp_listen_buf, sizeof(s_frudp_listen_buf),
                            0, &src_addr, &addrlen);
      frudp_rx(src_addr.sin_addr.s_addr, src_addr.sin_port,
               rxs->addr, rxs->port,
               s_frudp_listen_buf, nbytes);
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
  if (tx_len == sendto(g_frudp_tx_sock, tx_data, tx_len, 0,
                       (struct sockaddr *)(&g_frudp_tx_addr), 
                       sizeof(g_frudp_tx_addr)))
    return true;
  else
    return false;
}
