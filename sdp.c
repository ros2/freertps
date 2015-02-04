#include "freertps/freertps.h"
#include "freertps/sdp.h"
#include "freertps/udp.h"
#include <arpa/inet.h>

void freertps_sdp_rx(const in_addr_t src_addr, const in_port_t src_port,
                     const uint8_t *rx, const uint16_t len)
{
  FREERTPS_INFO("sdp rx %d bytes\n", len);
}

void freertps_sdp_init()
{
  FREERTPS_INFO("sdp init\n");
  freertps_udp_add_mcast_rx(inet_addr("239.255.0.1"), 7400, freertps_sdp_rx);
}

void freertps_sdp_fini()
{
  FREERTPS_INFO("sdp fini\n");
}
