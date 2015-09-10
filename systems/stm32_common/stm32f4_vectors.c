#include "metal/stack.h"
#include "startup.h"
#include "metal/arm_trap.h"

void unhandled_vector()
{
  arm_trap_unhandled_vector();
}

// declare weak symbols for all interrupt so they can be overridden easily
#define WEAK_VECTOR __attribute__((weak, alias("unhandled_vector")))
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

void dummy_reset_vector() { }

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

