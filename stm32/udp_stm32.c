#include "freertps/freertps.h"
#include "freertps/udp.h"
#include "freertps/discovery.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "net_config.h"
#include "enet.h"

bool frudp_init()
{
  FREERTPS_INFO("stm32 udp init()\n");
  FREERTPS_INFO("using address %d.%d.%d.%d for unicast\n", 
                (FRUDP_IP4_ADDR >> 24) & 0xff,
                (FRUDP_IP4_ADDR >> 16) & 0xff,
                (FRUDP_IP4_ADDR >>  8) & 0xff,
                (FRUDP_IP4_ADDR      ) & 0xff);
  frudp_generic_init();
  // not sure about endianness here.
  g_frudp_config.guid_prefix.prefix[0] = FREERTPS_VENDOR_ID >> 8;
  g_frudp_config.guid_prefix.prefix[1] = FREERTPS_VENDOR_ID & 0xff;
  memcpy(&g_frudp_config.guid_prefix.prefix[2], g_enet_mac, 6);
  // 4 bytes left. let's use the system time in microseconds since power-up
  // todo: init ethernet PHY. after PHY link is up, 
  // store system time in the guid_prefix. 
  //memcpy(&g_frudp_config.guid_prefix.prefix[8], &pid, 4);
  frudp_discovery_init();
  return true;
}

bool frudp_init_participant_id()
{
  g_frudp_config.participant_id = 0;
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
                            0,
                            (struct sockaddr *)&src_addr,
                            (socklen_t *)&addrlen);
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
  if (tx_len == sendto(g_frudp_rx_socks[3].sock, tx_data, tx_len, 0,
                       (struct sockaddr *)(&g_frudp_tx_addr),
                       sizeof(g_frudp_tx_addr)))
    return true;
  else
    return false;
}
