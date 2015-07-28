#include "freertps/system.h"
#include "freertps/udp.h"

void freertps_system_init()
{
  frudp_init();
  __enable_irq();
}

bool freertps_system_ok()
{
  return true;
}

