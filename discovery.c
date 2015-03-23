#include "freertps/discovery.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"

void frudp_discovery_init()
{
  FREERTPS_INFO("discovery init\n");
  frudp_spdp_init();
  frudp_sedp_init();
}

void frudp_discovery_fini()
{
  FREERTPS_INFO("discovery fini\n");
  frudp_spdp_fini();
  frudp_sedp_fini();
}

void frudp_discovery_tick()
{
  frudp_spdp_tick();
  frudp_sedp_tick();
}
