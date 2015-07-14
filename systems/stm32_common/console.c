#include "console.h"
#include "pin.h"
#include <stdbool.h>

static volatile bool g_console_init_complete = false;

void console_send_block(const uint8_t *buf, uint32_t len)
{
  if (!g_console_init_complete)
    console_init();
  for (uint32_t i = 0; i < len; i++)
  {
    while (!(g_console_usart->ISR & USART_ISR_TXE)) { } // wait for tx buffer
    g_console_usart->TDR = buf[i];
  }
  while (!(g_console_usart->ISR & USART_ISR_TC)) { } // wait for TX to finish
}

