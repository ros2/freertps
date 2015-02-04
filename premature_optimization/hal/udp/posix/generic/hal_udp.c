#include "freertps/freertps.h"
#include "freertps/udp.h"
#include <stdint.h>

void freertps_hal_udp_init()
{
  FREERTPS_INFO("HAL UDP init()\n");
}

void freertps_hal_udp_fini()
{
  FREERTPS_INFO("HAL UDP fini()\n");
}

void freertps_hal_udp_add_mcast_rx_port(uint16_t port)
{
  FREERTPS_INFO("add mcast rx port %d\n", port);
}

