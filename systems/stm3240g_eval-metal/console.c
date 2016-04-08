#include "metal/console.h"
#include "pin.h"
#include <stdbool.h>

// finding a spare UART tx pin on the STM3240G-eval board is tricky... just
// about every pin is doing several things. This configuration of UART means
// that you can't use the micro-SD card. Every other configuration I could
// find also has tradeoffs. At the moment I don't intend to use micro-SD
// with freertps-based systems, so we'll just go with this for now. This can
// be swapped for various other tradeoffs (i.e., make the camera unusable)
// if desired.
// 
// PC12 = UART5 TX on AF8

#define PORTC_TX_PIN 12

USART_TypeDef *g_console_usart = UART5;

static volatile bool g_console_init_complete = false;

void console_init(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
  pin_set_alternate_function(GPIOC, PORTC_TX_PIN, 8);
  g_console_usart->CR1 &= ~USART_CR1_UE;
  g_console_usart->CR1 |=  USART_CR1_TE;
  // we want 1 megabit. the UART5 bus is 168 mhz / 4 = 42 mhz.
  // the uart bit counter is /16, so we need to further divide by 2.625
  // to get 1 megabit using the fractional baud rate divider
  // so, we have to set mantissa=2 and fraction (sixteenths)=10
  g_console_usart->BRR  = (((uint16_t)2) << 4) | 10;
  g_console_usart->CR1 |=  USART_CR1_UE;
  g_console_init_complete = true;
}

void console_send_block(const uint8_t *buf, uint32_t len)
{
  if (!g_console_init_complete)
    console_init();
  for (uint32_t i = 0; i < len; i++)
  {
    while (!(g_console_usart->SR & USART_SR_TXE)) { } // wait for tx buffer
    g_console_usart->DR = buf[i];
  }
  while (!(g_console_usart->SR & USART_SR_TC)) { } // wait for TX to finish
}

