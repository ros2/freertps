/*  Software License Agreement (Apache License)
 *
 *  Copyright 2015 Open Source Robotics Foundation
 *  Author: Morgan Quigley
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdint.h>
#include "samv71q21.h"

extern uint32_t _sfixed, _efixed, _etext;
extern uint32_t _srelocate, _erelocate, _szero, _ezero;

extern int main();
extern void __libc_init_array();

void reset_vector()
{
  uint32_t *pSrc, *pDest;
  EFC->EEFC_FMR = EEFC_FMR_FWS(1); // slow down the flash a bit
  WDT->WDT_MR = WDT_MR_WDDIS; // disable watchdog for now
  // set up data segment
  pSrc = &_etext;
  pDest = &_srelocate;
  if (pSrc != pDest)
    for (; pDest < &_erelocate; )
      *pDest++ = *pSrc++;
  // set up bss segment
  for (pDest = &_szero; pDest < &_ezero; )
    *pDest++ = 0;
  // set vector table base address (if needed)
  pSrc = (uint32_t *)&_sfixed;
  SCB->VTOR = ( (uint32_t)pSrc & SCB_VTOR_TBLOFF_Msk ); // 7 LSB's are 0
  if (((uint32_t)pSrc >= IRAM_ADDR) && ((uint32_t)pSrc < IRAM_ADDR+IRAM_SIZE))
    SCB->VTOR |= 1 << 29; // TBLBASE bit 
  __libc_init_array();
  main(); // jump to application main()
  while (1) { } // hopefully we never get here...
}

