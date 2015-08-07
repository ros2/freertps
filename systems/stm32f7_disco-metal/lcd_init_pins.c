#include <stdio.h>
#include "pin.h"

// LCD Red pins
#define PORTI_LCD_R0 15
#define PORTJ_LCD_R1 0
#define PORTJ_LCD_R2 1
#define PORTJ_LCD_R3 2
#define PORTJ_LCD_R4 3
#define PORTJ_LCD_R5 4
#define PORTJ_LCD_R6 5
#define PORTJ_LCD_R7 6
//LCD Green pins
#define PORTJ_LCD_G0 7
#define PORTJ_LCD_G1 8
#define PORTJ_LCD_G2 9
#define PORTJ_LCD_G3 10
#define PORTJ_LCD_G4 11
#define PORTK_LCD_G5 0
#define PORTK_LCD_G6 1
#define PORTK_LCD_G7 2
//LCD Blue pins
#define PORTE_LCD_B0 4
#define PORTJ_LCD_B1 13
#define PORTJ_LCD_B2 14
#define PORTJ_LCD_B3 15
#define PORTG_LCD_B4 12
#define PORTK_LCD_B5 4
#define PORTK_LCD_B6 5
#define PORTK_LCD_B7 6
//LCD Control signals
#define PORTI_LCD_VSYNC 9
#define PORTI_LCD_HSYNC 10
#define PORTI_LCD_DISP 12
#define PORTI_LCD_CLK 14

#define PORTI_LCD_INT 13
#define PORTK_LCD_BL_CONTROL 3  // ??
#define PORTK_LCD_DE 7          // ??

//LCD Communication pins
//#define PORTH_LCD_SCL 7
//#define PORTH_LCD_SDA 8

// Alternate Function definition
#define AF_LCD 14
#define AF_LCD_BIS 9
#define AF_I2C 4

void lcd_init_pins()
{
//  #ifdef DEBUG
  printf("lcd_init_pins()\r\n");
//  #endif

// LCD Red pins
  pin_set_alternate_function(GPIOI, PORTI_LCD_R0, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R1, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R2, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R3, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R4, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R5, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R6, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_R7, AF_LCD);
//LCD Green pins
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_G0, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_G1, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_G2, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_G3, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_G4, AF_LCD);
  pin_set_alternate_function(GPIOK, PORTK_LCD_G5, AF_LCD);
  pin_set_alternate_function(GPIOK, PORTK_LCD_G6, AF_LCD);
  pin_set_alternate_function(GPIOK, PORTK_LCD_G7, AF_LCD);
//LCD Blue pins
  pin_set_alternate_function(GPIOE, PORTE_LCD_B0, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_B1, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_B2, AF_LCD);
  pin_set_alternate_function(GPIOJ, PORTJ_LCD_B3, AF_LCD);
  pin_set_alternate_function(GPIOG, PORTG_LCD_B4, AF_LCD_BIS);
  pin_set_alternate_function(GPIOK, PORTK_LCD_B5, AF_LCD);
  pin_set_alternate_function(GPIOK, PORTK_LCD_B6, AF_LCD);
  pin_set_alternate_function(GPIOK, PORTK_LCD_B7, AF_LCD);
//LCD Control signals
  pin_set_alternate_function(GPIOI, PORTI_LCD_VSYNC, AF_LCD);
  pin_set_alternate_function(GPIOI, PORTI_LCD_HSYNC, AF_LCD);
  pin_set_alternate_function(GPIOI, PORTI_LCD_CLK, AF_LCD);
  pin_set_alternate_function(GPIOI, PORTI_LCD_INT, AF_LCD); //FIXME they do it in the example, doesnt make sense since we already have a VSYNC
  pin_set_alternate_function(GPIOK, PORTK_LCD_DE, AF_LCD);
////LCD I2C
//  pin_set_alternate_function(GPIOH, PORTH_LCD_SCL, AF_I2C);
//  pin_set_alternate_function(GPIOH, PORTH_LCD_SDA, AF_I2C);
//  // set speed i2c
//  pin_set_output_type(GPIOH, PORTH_LCD_SCL, PIN_OUTPUT_TYPE_OPEN_DRAIN);
//  pin_set_output_type(GPIOH, PORTH_LCD_SDA, PIN_OUTPUT_TYPE_OPEN_DRAIN);
//  pin_set_output_speed(GPIOH, PORTH_LCD_SCL, 0);
//  pin_set_output_speed(GPIOH, PORTH_LCD_SDA, 0);
/*
NOT ALTERNATE FUNCTIONS
#define PORTI_LCD_DISP 12
#define PORTI_LCD_INT 13
#define PORTK_LCD_BL_CONTROL 3
*/

//  pin_set_output_type(GPIOK, PORTK_LCD_DE, PIN_OUTPUT_TYPE_PUSH_PULL);
  pin_set_output_type(GPIOI, PORTI_LCD_DISP, PIN_OUTPUT_TYPE_PUSH_PULL);
//  pin_set_output_type(GPIOI, PORTI_LCD_INT, PIN_OUTPUT_TYPE_PUSH_PULL);   //No idea if its necessary ??
  pin_set_output_type(GPIOK, PORTK_LCD_BL_CONTROL, PIN_OUTPUT_TYPE_PUSH_PULL);
  
  pin_set_output_high(GPIOI, PORTI_LCD_DISP);
  pin_set_output_high(GPIOK, PORTK_LCD_BL_CONTROL);
  pin_set_output_speed(GPIOE, PORTE_LCD_B0, 2);
//  pin_set_output_high(GPIOI, PORTI_LCD_INT);

}


void lcd_turn_on(){
  pin_set_output_high(GPIOI, PORTI_LCD_DISP);
  pin_set_output_high(GPIOK, PORTK_LCD_BL_CONTROL);
}

void lcd_turn_off(){
  pin_set_output_low(GPIOI, PORTI_LCD_DISP);
  pin_set_output_low(GPIOK, PORTK_LCD_BL_CONTROL);
}
 
 
