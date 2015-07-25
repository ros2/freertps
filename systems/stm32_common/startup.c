#include <stdint.h>
#include "led.h"
#include "delay.h"
#include "systime.h"
#include "console.h"
#include "flash.h"
#include "stack.h"
//#include "watchdog.h"

extern uint32_t _srelocate_flash, _srelocate, _erelocate, _ebss, _sbss;
extern int main();

void startup_clock_init_fail() { while (1) { } }

void reset_vector()
{
  //watchdog_reset_counter();
  g_stack[0] = 0; // need to put a reference in here to the stack array
                  // to make sure the linker brings it in. I'm sure there
                  // is a more elegant way to do this, but this seems to work
  // set up data segment
  uint32_t *pSrc = &_srelocate_flash;
  uint32_t *pDest = &_srelocate;
  if (pSrc != pDest)
    for (; pDest < &_erelocate; )
      *pDest++ = *pSrc++;
  // set up bss segment
  for (pDest = &_sbss; pDest < &_ebss; )
    *pDest++ = 0;
  //__libc_init_array() ;
  SCB->CPACR |= ((3UL << (10*2)) | (3UL << (11*2))); // activate the FPU
  // set up the clocking scheme
  RCC->CR |= 0x1; // ensure the HSI (internal) oscillator is on
  RCC->CFGR = 0; // ensure the HSI oscillator is the clock source
  RCC->CR &= 0xfef6ffff; // turn off the main PLL and HSE oscillator
  RCC->PLLCFGR = 0x24003010; // ensure PLLCFGR is at reset state
  RCC->CR &= 0xfffbffff; // reset HSEBYP (i.e., HSE is *not* bypassed)
  RCC->CIR = 0x0; // disable all RCC interrupts
  RCC->CR |= RCC_CR_HSEON; // enable HSE oscillator (off-chip crystal)
  for (volatile uint32_t i = 0; 
       i < 0x5000 /*HSE_STARTUP_TIMEOUT*/ && !(RCC->CR & RCC_CR_HSERDY); i++)
  { } // wait for either timeout or HSE to spin up
  flash_init();
  if (!(RCC->CR & RCC_CR_HSERDY))
    startup_clock_init_fail(); // go there and spin forever. BUH BYE
  RCC->APB1ENR |= RCC_APB1ENR_PWREN; // clock up the power controller
  //PWR->CR |= PWR_CR_VOS; // ensure the voltage regulator is at max beef
  //                       // this will let us run at 168 MHz without overdrive
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // set HCLK (AHB clock) to sysclock
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2; // set APB high-speed clock to sysclock/2
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4; // set APB  low-speed clock to sysclock/4
  // PLL_M sets up an input frequency of 1 MHz for the PLL's, as per DS p.141
  #define PLL_M (HSE_VALUE / 1000000)
  // PLL_N is the main multipler. this sets up a VCO frequency of 1 * N = 336
  #define PLL_N 336
  #define PLL_P   2
  // SYSCLK = PLL_VCO / PLL_P = 168 MHz
  #define PLL_Q   7
  // USB clock = PLL_VCO / PLL_Q = 48 MHz
  RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1)-1) << 16) |
                 (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);
  RCC->CR |= RCC_CR_PLLON; // start spinning up the PLL
  while (!(RCC->CR & RCC_CR_PLLRDY)) { } // wait until it's spun up
  RCC->CFGR &= ~((uint32_t)RCC_CFGR_SW); // select internal oscillator
  RCC->CFGR |= RCC_CFGR_SW_PLL; // select PLL as clock source
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { } // wait for it...
  // hooray we're done! we're now running at 168 MHz.
  systime_init();
  led_init();
  console_init();
  main(); // jump to application main()
  while (1) { } // hopefully we never get here...
}

//#define STACK_SIZE 0x4000
//__attribute__((aligned(8),section(".stack"))) uint8_t g_stack[STACK_SIZE];

