#include "flash.h"

void flash_init(void)
{
  FLASH->ACR = 0; // ensure the caches are turned off, so we can reset them
  FLASH->ACR = FLASH_ACR_PRFTEN |  // enable flash prefetch
               FLASH_ACR_ARTEN |   // enable ART (flash accelerator)
               FLASH_ACR_LATENCY_5WS; // set 5-wait-state 
}
