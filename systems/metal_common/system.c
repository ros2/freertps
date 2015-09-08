#include "freertps/system.h"
#include "freertps/udp.h"
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

