#include "console.h"
#include "stm32f427xx.h"
#include "pin.h"
#include <stdbool.h>

// pin connections
// PE0 = UART8 RX on AF8
// PE1 = UART8 TX on AF8

#define PORTE_RX_PIN 0
#define PORTE_TX_PIN 1

static volatile bool s_console_init_complete = false;
static volatile USART_TypeDef * const s_console_usart = UART8; // todo

void console_init()
{
  s_console_init_complete = true;
  RCC->APB1ENR |= RCC_APB1ENR_UART8EN;
  pin_set_alternate_function(GPIOE, PORTE_RX_PIN, 8);
  pin_set_alternate_function(GPIOE, PORTE_TX_PIN, 8);
  s_console_usart->CR1 &= ~USART_CR1_UE;
  s_console_usart->CR1 |=  USART_CR1_TE | USART_CR1_RE;
  // we want 1 megabit. do this with mantissa=3 and fraction (sixteenths)=0
  s_console_usart->BRR  = (((uint16_t)2) << 4) | 10;
  s_console_usart->CR1 |=  USART_CR1_UE;
}

void console_send_block(const uint8_t *buf, uint32_t len)
{
  if (!s_console_init_complete)
    console_init();
  for (uint32_t i = 0; i < len; i++)
  {
    while (!(s_console_usart->SR & USART_SR_TXE)) { } // wait for tx buffer to clear
    s_console_usart->DR = buf[i];
  }
  while (!(s_console_usart->SR & USART_SR_TC)) { } // wait for TX to finish
}

