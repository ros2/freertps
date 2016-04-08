#include "freertps/timer.h"
#include <stdlib.h>

static freertps_timer_cb_t g_freertps_timer_cb = NULL;

void freertps_timer_set_freq(uint32_t freq, freertps_timer_cb_t cb)
{
  // set up TIM5 as the timer. generalize this someday if needed
  RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
  TIM5->PSC = 168000000 / 2 / 1000000 - 1; // microsecond resolution
  if (freq == 0 || freq >= 500000)
    while (1) { } // extreme badness
  TIM5->ARR = 1000000 / freq - 1; // auto-reload @ 1000 Hz
  TIM5->EGR = TIM_EGR_UG; // load PSC immediately
  TIM5->CR1 = TIM_CR1_CEN; // start it counting
  TIM5->DIER = TIM_DIER_UIE; // enable update interrupt
  NVIC_SetPriority(TIM5_IRQn, 2);
  NVIC_EnableIRQ(TIM5_IRQn);
  g_freertps_timer_cb = cb;
}

void tim5_vector(void)
{
  TIM5->SR &= ~TIM_SR_UIF; // clear the update flag
  if (g_freertps_timer_cb)
    g_freertps_timer_cb();
}

