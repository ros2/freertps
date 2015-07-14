#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

extern USART_TypeDef *g_console_usart;

void console_init();
void console_send_block(const uint8_t *buf, uint32_t len);

#endif

