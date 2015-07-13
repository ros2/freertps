#include "freertps/system.h"
#include "systems/stm32_common/cmsis/stm32f407.h"

void freertps_system_init()
{
  frudp_init();
  __enable_irq();
}

bool freertps_system_ok()
{
    return true;
}

