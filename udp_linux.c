#include "freertps/freertps.h"
#include "freertps/udp.h"
#include <stdint.h>

void freertps_udp_init()
{
  FREERTPS_INFO("udp init()\n");
  freertps_sdp_init();
}

void freertps_udp_fini()
{
  FREERTPS_INFO("udp fini()\n");
  freertps_sdp_fini();
}

void freertps_udp_add_mcast_rx_port(uint16_t port)
{
  FREERTPS_INFO("add mcast rx port %d\n", port);
}

