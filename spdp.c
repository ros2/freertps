#include "freertps/freertps.h"
#include "freertps/spdp.h"
#include "freertps/udp.h"
#include <arpa/inet.h>

void freertps_spdp_rx(const in_addr_t src_addr, const in_port_t src_port,
                      const uint8_t *rx, const uint16_t len)
{
}

void freertps_spdp_init()
{
  FREERTPS_INFO("sdp init\n");
}

void freertps_spdp_fini()
{
  FREERTPS_INFO("sdp fini\n");
}
