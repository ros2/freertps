#include "console.h"
#include "stm32f746xx.h"
#include "pin.h"
#include <stdbool.h>

// pin connections for stm32f7-discovery board
// PC6 = UART6 TX on AF8
// PC7 = UART6 RX on AF8

#define PORTC_TX_PIN 6
#define PORTC_RX_PIN 7

static volatile bool s_console_init_complete = false;
static USART_TypeDef *s_console_usart = USART6;

void console_init()
{
  s_console_init_complete = true;
  RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
  pin_set_alternate_function(GPIOC, PORTC_RX_PIN, 8);
  pin_set_alternate_function(GPIOC, PORTC_TX_PIN, 8);
  s_console_usart->CR1 &= ~USART_CR1_UE;
  s_console_usart->CR1 |=  USART_CR1_TE | USART_CR1_RE;
  // we want 1 megabit. do this with mantissa=5 and fraction (sixteenths)=4
  s_console_usart->BRR  = (((uint16_t)5) << 4) | 4;
  s_console_usart->CR1 |=  USART_CR1_UE;
}

void console_send_block(const uint8_t *buf, uint32_t len)
{
  if (!s_console_init_complete)
    console_init();
  for (uint32_t i = 0; i < len; i++)
  {
    while (!(s_console_usart->ISR & USART_ISR_TXE)) { } // wait for tx buffer to clear
    s_console_usart->TDR = buf[i];
  }
  while (!(s_console_usart->ISR & USART_ISR_TC)) { } // wait for TX to finish
}

