#ifndef CAMERA_H
#define CAMERA_H

#include "i2c.h"

void lcd_init(uint32_t* bufferAddress);

#define BUFFER_SIZE 0x2850
static uint32_t aDST_Buffer[BUFFER_SIZE];

// BSP Functions
void lcd_init_pins();  // provided in a board-specific file
void lcd_turn_on();
void lcd_turn_off();
#endif
