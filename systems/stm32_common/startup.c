#include <stdint.h>
#include "freertps/periph/led.h"
#include "metal/delay.h"
#include "metal/systime.h"
#include "metal/console.h"
#include "flash.h"
#include "metal/stack.h"
//#include "watchdog.h"
#include <stdio.h>

void __libc_init_array(void); // apparently this isn't defined in a newlib header?

extern uint32_t _srelocate_flash, _srelocate, _erelocate, _ebss, _sbss;
extern int main(void);

void startup_clock_init_fail(void) { while (1) { } }

void reset_vector(void)
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
  __libc_init_array();
  SCB->CPACR |= ((3UL << (10*2)) | (3UL << (11*2))); // activate the FPU
  //TODO used define provided if changes in future generations ? 
  // set up the clocking scheme
  RCC->CR |= 0x1; // ensure the HSI (internal) oscillator is on
//  RCC->CR |= RCC_CR_HSION;
  RCC->CFGR = 0; // ensure the HSI oscillator is the clock source
//RCC->CFGR = RCC_CFGR_SW_HSI;
  RCC->CR &= 0xfef6ffff; // turn off the main PLL and HSE oscillator
// RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_CSSON);
  RCC->PLLCFGR = 0x24003010; // ensure PLLCFGR is at reset state
  RCC->CR &= 0xfffbffff; // reset HSEBYP (i.e., HSE is *not* bypassed)
//  RCC->CR &= ~RCC_CR_HSEBYP;
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
  static char metal_stdout_buf[1024];
  setvbuf(stdout, metal_stdout_buf, _IOLBF, sizeof(metal_stdout_buf));
  systime_init();
  led_init();
  console_init();
  main(); // jump to application main()
  while (1) { } // hopefully we never get here...
}

//#define STACK_SIZE 0x4000
//__attribute__((aligned(8),section(".stack"))) uint8_t g_stack[STACK_SIZE];


/* ENABLE OVERDRIVE MODE*/
//1) Select HSE as sys clock (in RCC_CR) => HSEON, HSERDY. RCC_CFGR-> SW = 01 
//   (select HSE) then wait for sys clock to be ready (rcc_cr->sws)
//2) config RCC_PLLCFCG and set PLLON in RCC_CR
//3) Set ODEN in PWR_CR1 and wait for ODRDY in PWR_CSR1
//4) set ODSW in PWR_CR1 and wait for ODSWRDY in PWR_CSR1
//5) select flash latecny ?? and prescalers
//6) wait for pll lock in RCC_CR
//7) switch system to pll RCC->CR SW = 10 for pll then wait for SWS to return 10
//8) enable all peripherals
//
//void enable_overdrive(void){
//  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
//  //1
//  RCC->CR |= RCC_CR_HSEON;                // turn on HSE if not already done
//  while((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY);   //wait for HSE Ready
//  RCC->CFGR &= ~RCC_CFGR_SW;              // Select HSE as System clock
//  RCC->CFGR |= RCC_CFGR_SW_0;
//  while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0);  // Wait for system 
//                                                        //clock ready on HSE
//  //2
//  RCC->PLLCFGR = (PLL_Q << 24) |  (RCC_PLLCFGR_PLLSRC_HSE)| 
//         (((PLL_P >> 1)-1) << 16) | (PLL_N << 6) | PLL_M;
//  RCC->CR |= RCC_CR_PLLON;
//  //3
//  PWR->CR1 |= PWR_CR1_ODEN;
//  while((PWR->CSR1 & PWR_CSR1_ODRDY)!= PWR_CSR1_ODRDY);
//  //4
//  PWR->CR1 |= PWR_CR1_ODSWEN;
//  while((PWR->CSR1 & PWR_CSR1_ODSWRDY)!= PWR_CSR1_ODSWRDY);
//  //5
//  RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // set HCLK (AHB clock) to sysclock
//  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2; // set APB high-speed clock to sysclock/2
//  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4; // set APB  low-speed clock to sysclock/4
////FIXME check optimal number of wait state according to configuration
//  FLASH->ACR = ((FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_7WS);
//  //6
//  while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY);
//  //7
//  RCC->CFGR = ((RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL);//select PLL as CLK
//  while((RCC->CFGR & RCC_CFGR_SWS) | RCC_CFGR_SWS_PLL);// wait clock switch
//}
//
//void disable_overdrive(void){
////1) Select HSE as sys clock (in RCC_CR) => HSEON, HSERDY. RCC_CFGR-> SW = 01 
////   (select HSE) then wait for sys clock to be ready (rcc_cr->sws)
////2) store previous apb1 apb2 registers configurations
////3) disable clocks generated by pll others thant main pll
////4) reset ODSW and ODEN in PWR->CR
////5) wait for ODWRDY
////7) reenable peripheral clocks
//
//  //1
//  RCC->CR |= RCC_CR_HSEON;                // turn on HSE if not already done
//  while((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY);   //wait for HSE Ready
//  RCC->CFGR &= ~RCC_CFGR_SW;              // Select HSE as System clock
//  RCC->CFGR |= RCC_CFGR_SW_0;
//  while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0);  // Wait for system 
//                                                        //clock ready on HSE
//  uint32_t ahb2_temp= RCC->AHB2ENR, apb1_temp=RCC->APB1ENR,apb2_temp=RCC->APB2ENR;
////  RCC->APB1ENR =0; RCC->APB2ENR =0; 
//  RCC->AHB2ENR &= ~(RCC_AHB2LPENR_OTGFSLPEN | RCC_AHB2LPENR_RNGLPEN);
//  RCC->APB1ENR &= ~(RCC_APB1ENR_SPDIFRXEN);
//  RCC->APB2ENR &= ~(RCC_APB2ENR_SAI1EN | RCC_APB2ENR_SAI2EN | 
//                    RCC_APB2ENR_LTDCEN | RCC_APB2ENR_SDMMC1EN);
//
//  PWR->CR1 &= ~PWR_CR1_ODSWEN   &
//              ~PWR_CR1_ODEN     ;
//  while((PWR->CSR1 & PWR_CSR1_ODSWRDY) != 0);
//  RCC->APB1ENR = apb1_temp;
//  RCC->APB2ENR = apb2_temp;
//  RCC->AHB2ENR = ahb2_temp;
//
//}
