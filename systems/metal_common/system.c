#include "freertps/system.h"
#include "freertps/udp.h"
#include "freertps/freertps.h"
#include "metal.h"

void freertps_system_init()
{
  frudp_init();
  frudp_generic_init();
  freertps_metal_enable_irq();
}

bool freertps_system_ok()
{
  return true;
}

bool frudp_init_participant_id()
{
  FREERTPS_INFO("frudp_init_participant_id()\n");
  g_frudp_config.participant_id = 0;
  return true;
}
