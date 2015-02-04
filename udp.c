#include "freertps/udp.h"

void freertps_udp_init()
{
  freertps_hal_udp_init();
  freertps_sdp_init();
}

void freertps_udp_fini()
{
  freertps_sdp_fini();
  freertps_hal_udp_fini();
}
