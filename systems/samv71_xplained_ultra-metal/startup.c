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
#include "metal/stack.h"
#include <stdio.h>
#include "metal/systime.h"
#include "metal/console.h"
#include "freertps/periph/led.h"

extern uint32_t _sfixed, _efixed, _etext;
extern uint32_t _srelocate, _erelocate, _szero, _ezero;

extern int main(void);
extern void __libc_init_array(void);
//extern void _mainCRTStartup(void);

void wdt_vector(void)
{
  while (1);
}

// from Atmel SAMV71 doc, section 3.2.1, "Enabling the FPU"
#define ADDR_CPACR 0xe000ed88
#define REG_CPACR (*((volatile uint32_t *)ADDR_CPACR))
static void enable_fpu(void)
{
  REG_CPACR |= 0xfu << 20;
}

void reset_vector(void)
{
  g_stack[0] = 0; // need to put a reference in here to the stack array
                  // to make sure the linker brings it in. I'm sure there
                  // is a more elegant way to do this, but this seems to work
  EFC->EEFC_FMR = EEFC_FMR_FWS(5); // slow down flash for our blazing speed
  WDT->WDT_MR = WDT_MR_WDDIS; // disable watchdog for now
  // TODO: a block of code which can be ifdef'd in and out to source the
  // slow clock from a 32 kHz crystal rather than the (relatively) inaccurate
  // internal RC oscillator
  PMC->PMC_MCKR = (PMC->PMC_MCKR & ~(uint32_t)PMC_MCKR_CSS_Msk) |
                  PMC_MCKR_CSS_MAIN_CLK;
  PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD |
                  CKGR_MOR_MOSCXTST(0x10) | // startup time: slowclock*8*this
                  CKGR_MOR_MOSCRCEN | // keep main on-chip RC oscillator on !
                  CKGR_MOR_MOSCXTEN; // crystal oscillator enable (not select)
  while (!(PMC->PMC_SR & PMC_SR_MOSCSELS)) { } // spin until stable
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY)) { } // spin until selected
  PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD |     // "password" hard-wired in logic
                  CKGR_MOR_MOSCXTST(0x10) | // startup time: slowclock*8*this
                  CKGR_MOR_MOSCRCEN | // keep main on-chip RC oscillator on !
                  CKGR_MOR_MOSCXTEN; // main crystal oscillator enable
  while (!(PMC->PMC_SR & PMC_SR_MOSCXTS)) { } // busy wait
  // switch to main crystal oscillator
  PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD     |
                  CKGR_MOR_MOSCXTST(0x10) |
                  CKGR_MOR_MOSCRCEN       | // keep on-chip RC oscillator on !
                  CKGR_MOR_MOSCXTEN       |
                  CKGR_MOR_MOSCSEL;
  while (!(PMC->PMC_SR & PMC_SR_MOSCSELS) ||
         !(PMC->PMC_SR & PMC_SR_MCKRDY)       ) { } // spin until stable
  PMC->PMC_MCKR = (PMC->PMC_MCKR & ~(uint32_t)PMC_MCKR_CSS_Msk) |
                  PMC_MCKR_CSS_MAIN_CLK;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY)) { } // spin until selected
  // now, let's measure the frequency of the main crystal oscillator
  PMC->CKGR_MCFR = CKGR_MCFR_CCSS   | // measure the crystal oscillator
                   CKGR_MCFR_RCMEAS ; // start a new measurement
  // PLLA must output between 150 MHz and 500 MHz
  // board has 12 MHz crystal; let's multiply by 24 for 288 MHz PLL freq
  #define MUL 23
  PMC->CKGR_PLLAR = CKGR_PLLAR_ONE         | // per datasheet, must set 1<<29
                    CKGR_PLLAR_MULA(MUL)   | // pll = crystal * (mul+1)/div
                    CKGR_PLLAR_DIVA(1)     |
                    CKGR_PLLAR_PLLACOUNT(0x3f);
  while (!(PMC->PMC_SR & PMC_SR_LOCKA)) { } // spin until lock
  // don't use a divider... use the PLL output as CPU clock and divide CPU
  // clock by 2 to get 144 MHz for the master clock
  PMC->PMC_MCKR =  PMC_MCKR_CSS_MAIN_CLK | // | 
                   PMC_MCKR_MDIV_PCK_DIV2;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY)) { } // spin until ready
  // finally, dividers are all set up, so let's switch CPU to the PLLA output
  PMC->PMC_MCKR =  PMC_MCKR_CSS_PLLA_CLK | 
                   PMC_MCKR_MDIV_PCK_DIV2;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY)) { } // spin until selected
  // now we're running the CPU at 288 MHz and the system at 144 MHz

  uint32_t *pSrc, *pDest;
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
  enable_fpu();
  __libc_init_array();
  static char metal_stdout_buf[1024];
  setvbuf(stdout, metal_stdout_buf, _IOLBF, sizeof(metal_stdout_buf));
  systime_init();
  led_init();
  console_init();
  main();
  while (1) { } // hopefully we never get here...
}
