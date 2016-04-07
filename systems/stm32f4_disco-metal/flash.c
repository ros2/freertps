#include "flash.h"

void flash_init(void)
{
  FLASH->ACR = 0; // ensure the caches are turned off, so we can reset them
  FLASH->ACR = FLASH_ACR_DCRST | FLASH_ACR_ICRST; // flush the cache
  FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |
               FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS; // re-enable the caches
}
