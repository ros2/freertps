#include <stdint.h>
#include "stm32f427xx.h"
//#include "watchdog.h"

extern uint32_t _srelocate_flash, _srelocate, _erelocate, _ebss, _sbss;
extern int main();

void startup_clock_init_fail() { while (1) { } }

void reset_vector()
{
  //watchdog_reset_counter();
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
  FLASH->ACR = 0; // ensure the caches are turned off, so we can reset them
  FLASH->ACR = FLASH_ACR_DCRST | FLASH_ACR_ICRST; // flush the cache
  FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | 
               FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS; // re-enable the caches
  if (!(RCC->CR & RCC_CR_HSERDY))
    startup_clock_init_fail(); // go there and spin forever. BUH BYE
  RCC->APB1ENR |= RCC_APB1ENR_PWREN; // clock up the power controller
  PWR->CR |= PWR_CR_VOS; // ensure the voltage regulator is at max beef
                         // this will let us run at 168 MHz without overdrive
  RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // set HCLK (AHB clock) to sysclock
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2; // set APB high-speed clock to sysclock/2
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4; // set APB  low-speed clock to sysclock/4
  #define PLL_M (HSE_VALUE/1000000)
  #define PLL_N 336
  // PLL_VCO = crystal mhz / PLL_M * PLL_N = 336 MHz
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
  // hooray we're done! we're now running at 96 MHz.
  main(); // jump to application main()
  while (1) { } // hopefully we never get here...
}

void unmapped_vector()
{
  while (1) { } // spin to allow jtag trap
}

#define STACK_SIZE 0x4000
__attribute__((aligned(8),section(".stack"))) uint8_t g_stack[STACK_SIZE];

// declare weak symbols for all interrupt so they can be overridden easily
#define WEAK_VECTOR __attribute__((weak, alias("unmapped_vector")))
void nmi_vector() WEAK_VECTOR;
void hardfault_vector() WEAK_VECTOR;
void memmanage_vector() WEAK_VECTOR;
void busfault_vector() WEAK_VECTOR;
void usagefault_vector() WEAK_VECTOR;
void svcall_vector() WEAK_VECTOR;
void dbgmon_vector() WEAK_VECTOR;
void pendsv_vector() WEAK_VECTOR;
void systick_vector() WEAK_VECTOR;
void wwdg_vector() WEAK_VECTOR;
void pvd_vector() WEAK_VECTOR;
void tampstamp_vector() WEAK_VECTOR;
void rtc_wkup_vector() WEAK_VECTOR;
void flash_vector() WEAK_VECTOR;
void rcc_vector() WEAK_VECTOR;
void exti0_vector() WEAK_VECTOR;
void exti1_vector() WEAK_VECTOR;
void exti2_vector() WEAK_VECTOR;
void exti3_vector() WEAK_VECTOR;
void exti4_vector() WEAK_VECTOR;
void dma1_stream0_vector() WEAK_VECTOR;
void dma1_stream1_vector() WEAK_VECTOR;
void dma1_stream2_vector() WEAK_VECTOR;
void dma1_stream3_vector() WEAK_VECTOR;
void dma1_stream4_vector() WEAK_VECTOR;
void dma1_stream5_vector() WEAK_VECTOR;
void dma1_stream6_vector() WEAK_VECTOR;
void adc_vector() WEAK_VECTOR;
void can1_tx_vector() WEAK_VECTOR;
void can1_rx0_vector() WEAK_VECTOR;
void can1_rx1_vector() WEAK_VECTOR;
void can1_sce_vector() WEAK_VECTOR;
void exti9_5_vector() WEAK_VECTOR;
void tim1brk_tim9_vector() WEAK_VECTOR;
void tim1up_tim10_vector() WEAK_VECTOR;
void tim1trg_tim11_vector() WEAK_VECTOR;
void tim1cc_vector() WEAK_VECTOR;
void tim2_vector() WEAK_VECTOR;
void tim3_vector() WEAK_VECTOR;
void tim4_vector() WEAK_VECTOR;
void i2c1_ev_vector() WEAK_VECTOR;
void i2c1_er_vector() WEAK_VECTOR;
void i2c2_ev_vector() WEAK_VECTOR;
void i2c2_er_vector() WEAK_VECTOR;
void spi1_vector() WEAK_VECTOR;
void spi2_vector() WEAK_VECTOR;
void usart1_vector() WEAK_VECTOR;
void usart2_vector() WEAK_VECTOR;
void usart3_vector() WEAK_VECTOR;
void exti15_10_vector() WEAK_VECTOR;
void rtc_alarm_vector() WEAK_VECTOR;
void otg_fs_wkup_vector() WEAK_VECTOR;
void tim8brk_tim12_vector() WEAK_VECTOR;
void tim8up_tim13_vector() WEAK_VECTOR;
void tim8trg_tim14_vector() WEAK_VECTOR;
void tim8cc_vector() WEAK_VECTOR;
void dma1_stream7_vector() WEAK_VECTOR;
void fsmc_vector() WEAK_VECTOR;
void sdio_vector() WEAK_VECTOR;
void tim5_vector() WEAK_VECTOR;
void spi3_vector() WEAK_VECTOR;
void uart4_vector() WEAK_VECTOR;
void uart5_vector() WEAK_VECTOR;
void tim6_dac_vector() WEAK_VECTOR;
void tim7_vector() WEAK_VECTOR;
void dma2_stream0_vector() WEAK_VECTOR;
void dma2_stream1_vector() WEAK_VECTOR;
void dma2_stream2_vector() WEAK_VECTOR;
void dma2_stream3_vector() WEAK_VECTOR;
void dma2_stream4_vector() WEAK_VECTOR;
void eth_vector() WEAK_VECTOR;
void eth_wkup_vector() WEAK_VECTOR;
void can2_tx_vector() WEAK_VECTOR;
void can2_rx0_vector() WEAK_VECTOR;
void can2_rx1_vector() WEAK_VECTOR;
void can2_sce_vector() WEAK_VECTOR;
void otg_fs_vector() WEAK_VECTOR;
void dma2_stream5_vector() WEAK_VECTOR;
void dma2_stream6_vector() WEAK_VECTOR;
void dma2_stream7_vector() WEAK_VECTOR;
void usart6_vector() WEAK_VECTOR;
void i2c3_ev_vector() WEAK_VECTOR;
void i2c3_er_vector() WEAK_VECTOR;
void otg_hs_ep1_out_vector() WEAK_VECTOR;
void otg_hs_ep1_in_vector() WEAK_VECTOR;
void otg_hs_wkup_vector() WEAK_VECTOR;
void otg_hs_vector() WEAK_VECTOR;
void dcmi_vector() WEAK_VECTOR;
void cryp_vector() WEAK_VECTOR;
void hash_rng_vector() WEAK_VECTOR;
void fpu_vector() WEAK_VECTOR;
void uart7_vector() WEAK_VECTOR;
void uart8_vector() WEAK_VECTOR;
void spi4_vector() WEAK_VECTOR;
void spi5_vector() WEAK_VECTOR;
void spi6_vector() WEAK_VECTOR;
void sai1_vector() WEAK_VECTOR;
void ltdc_vector() WEAK_VECTOR;
void ltdc_er_vector() WEAK_VECTOR;
void dma2d_vector() WEAK_VECTOR;

typedef void (*vector_func_t)();
__attribute__((section(".vectors"))) vector_func_t g_vectors[] =
{
    (vector_func_t)(&g_stack[STACK_SIZE-8]), // initial stack pointer
    reset_vector,
    nmi_vector,
    hardfault_vector,
    memmanage_vector,
    busfault_vector,
    usagefault_vector,
    0, 0, 0, 0,  
    svcall_vector,
    dbgmon_vector,
    0,                                      
    pendsv_vector,
    systick_vector,
    wwdg_vector,       // 0
    pvd_vector,        
    tampstamp_vector,
    rtc_wkup_vector,
    flash_vector,
    rcc_vector,
    exti0_vector,
    exti1_vector,
    exti2_vector,
    exti3_vector,
    exti4_vector,      // 10
    dma1_stream0_vector,
    dma1_stream1_vector,
    dma1_stream2_vector,
    dma1_stream3_vector,
    dma1_stream4_vector,
    dma1_stream5_vector,
    dma1_stream6_vector,
    adc_vector,
    can1_tx_vector,
    can1_rx0_vector,   // 20
    can1_rx1_vector,
    can1_sce_vector,
    exti9_5_vector,
    tim1brk_tim9_vector,
    tim1up_tim10_vector,
    tim1trg_tim11_vector,
    tim1cc_vector,
    tim2_vector,
    tim3_vector,
    tim4_vector,
    i2c1_ev_vector,
    i2c1_er_vector,
    i2c2_ev_vector,
    i2c2_er_vector,
    spi1_vector,
    spi2_vector,
    usart1_vector,
    usart2_vector,
    usart3_vector,
    exti15_10_vector,
    rtc_alarm_vector,
    otg_fs_wkup_vector,
    tim8brk_tim12_vector,
    tim8up_tim13_vector,
    tim8trg_tim14_vector,
    tim8cc_vector,
    dma1_stream7_vector,
    fsmc_vector,
    sdio_vector,
    tim5_vector,
    spi3_vector,
    uart4_vector,
    uart5_vector,
    tim6_dac_vector,
    tim7_vector,
    dma2_stream0_vector,
    dma2_stream1_vector,
    dma2_stream2_vector,
    dma2_stream3_vector,
    dma2_stream4_vector,
    eth_vector,
    eth_wkup_vector,
    can2_tx_vector,
    can2_rx0_vector,
    can2_rx1_vector,
    can2_sce_vector,
    otg_fs_vector,
    dma2_stream5_vector,
    dma2_stream6_vector,
    dma2_stream7_vector,
    usart6_vector,
    i2c3_ev_vector,
    i2c3_er_vector,
    otg_hs_ep1_out_vector,
    otg_hs_ep1_in_vector,
    otg_hs_wkup_vector,
    otg_hs_vector,
    dcmi_vector,
    cryp_vector,
    hash_rng_vector,
    fpu_vector,
    uart7_vector,
    uart8_vector,
    spi4_vector,
    spi5_vector,
    spi6_vector,
    sai1_vector,
    ltdc_vector,
    ltdc_er_vector,
    dma2d_vector
};
