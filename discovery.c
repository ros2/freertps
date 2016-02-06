#include "freertps/discovery.h"
#include "freertps/freertps.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"

void fr_discovery_init()
{
  FREERTPS_INFO("discovery_init()\r\n");
  fr_spdp_init();
  fr_sedp_init();
}

void fr_discovery_fini()
{
  FREERTPS_INFO("discovery_fini()\r\n");
  fr_spdp_fini();
  fr_sedp_fini();
}

void fr_discovery_tick()
{
  fr_spdp_tick();
  fr_sedp_tick();
}
