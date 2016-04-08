#include "metal/systime.h"
#include "metal/delay.h"

void systime_init(void)
{
  // todo; use TIM2 since it's a 32-bit counter. just have it count
  // microseconds since powerup or something.
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  for (volatile int i = 0; i < 10000; i++) { }
  TIM2->PSC = 168000000 / 2 / 1000000 - 1; // 83
  TIM2->ARR = 0xffffffff; // count as long as possible
  TIM2->EGR = TIM_EGR_UG; // load the PSC register immediately
  TIM2->CR1 = TIM_CR1_CEN; // start counter
}

uint32_t systime_usecs(void)
{
  return TIM2->CNT;
}
