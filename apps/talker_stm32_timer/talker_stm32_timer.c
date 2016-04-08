#include <stdio.h>
#include "freertps/freertps.h"
#include <string.h>

frudp_pub_t *g_pub = NULL;

void tim5_vector(void)
{
  TIM5->SR &= ~TIM_SR_UIF; // clear the update flag
  if (!g_pub)
    return;
  static char __attribute__((aligned(4))) msg[256] = {0};
  static int pub_count = 0;
  snprintf(&msg[4], sizeof(msg), "Hello World: %d", pub_count++);
  uint32_t rtps_string_len = strlen(&msg[4]) + 1;
  *((uint32_t *)msg) = rtps_string_len;
  freertps_publish(g_pub, (uint8_t *)msg, rtps_string_len + 4);
}

int main(int argc, char **argv)
{
  // set up TIM5 to be our tx timer
  RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
  TIM5->PSC = 168000000 / 2 / 1000000 - 1; // microsecond resolution
  TIM5->ARR = 1000 - 1; // auto-reload @ 1000 Hz
  TIM5->EGR = TIM_EGR_UG; // load PSC immediately
  TIM5->CR1 = TIM_CR1_CEN; // start it counting
  TIM5->DIER = TIM_DIER_UIE; // enable update interrupt
  NVIC_SetPriority(TIM5_IRQn, 2);
  NVIC_EnableIRQ(TIM5_IRQn);

  printf("hello, world!\r\n");
  freertps_system_init();
  g_pub = freertps_create_pub
            ("chatter", "std_msgs::msg::dds_::String_");
  while (freertps_system_ok())
  {
    frudp_listen(1000000);
    frudp_disco_tick();
    //printf("sending: [%s]\n", &msg[4]);
  }
  frudp_fini();
  return 0;
}

