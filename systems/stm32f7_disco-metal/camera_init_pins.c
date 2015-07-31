#include <stdio.h>
#include "pin.h"

// DCMI mode GPIO
#define PORTA_DCMI_PIXCLK 6

#define PORTA_DCMI_HSYNC 4
#define PORTG_DCMI_VSYNC 9

#define PORTH_DCMI_D0 9
#define PORTH_DCMI_D1 10
#define PORTH_DCMI_D2 11
#define PORTH_DCMI_D3 12
#define PORTH_DCMI_D4 14
#define PORTD_DCMI_D5 3
#define PORTE_DCMI_D6 5
#define PORTE_DCMI_D7 6

// Standard GPIO
#define PORTH_DCMI_PWR_EN 13
#define DCMI_PWR_PIN 13
#define DCMI_PWR_IOBANK GPIOH
// I2C pins
#define PORTB_DCMI_SCL 8
#define PORTB_DCMI_SDA 9

// Alternate Function definition
#define AF_DCMI 13
#define AF_I2C 4

#define DCMI_DATA_WIDTH 8

void camera_init_pins()
{
  printf("camera_init_pins()\r\n");

  pin_set_alternate_function(GPIOA, PORTA_DCMI_PIXCLK, AF_DCMI);

  pin_set_alternate_function(GPIOA, PORTA_DCMI_HSYNC, AF_DCMI);
  pin_set_alternate_function(GPIOG, PORTG_DCMI_VSYNC, AF_DCMI);

  pin_set_alternate_function(GPIOH, PORTH_DCMI_D0, AF_DCMI);
  pin_set_alternate_function(GPIOH, PORTH_DCMI_D1, AF_DCMI);
  pin_set_alternate_function(GPIOH, PORTH_DCMI_D2, AF_DCMI);
  pin_set_alternate_function(GPIOH, PORTH_DCMI_D3, AF_DCMI);
  pin_set_alternate_function(GPIOH, PORTH_DCMI_D4, AF_DCMI);
  pin_set_alternate_function(GPIOD, PORTD_DCMI_D5, AF_DCMI);
  pin_set_alternate_function(GPIOE, PORTE_DCMI_D6, AF_DCMI);
  pin_set_alternate_function(GPIOE, PORTE_DCMI_D7, AF_DCMI);

  
  pin_set_output_speed(GPIOA, PORTA_DCMI_PIXCLK, 3);
  pin_set_output_speed(GPIOA, PORTA_DCMI_HSYNC, 3);
  pin_set_output_speed(GPIOG, PORTG_DCMI_VSYNC, 3);


  pin_set_output_speed(GPIOH, PORTH_DCMI_D0, 3);
  pin_set_output_speed(GPIOH, PORTH_DCMI_D1, 3);
  pin_set_output_speed(GPIOH, PORTH_DCMI_D2, 3);
  pin_set_output_speed(GPIOH, PORTH_DCMI_D3, 3);
  pin_set_output_speed(GPIOH, PORTH_DCMI_D4, 3);
  pin_set_output_speed(GPIOD, PORTD_DCMI_D5, 3);
  pin_set_output_speed(GPIOE, PORTE_DCMI_D6, 3);
  pin_set_output_speed(GPIOE, PORTE_DCMI_D7, 3);

  pin_set_alternate_function(GPIOB, PORTB_DCMI_SCL, AF_I2C);
  pin_set_alternate_function(GPIOB, PORTB_DCMI_SDA, AF_I2C);

  pin_set_output_state(GPIOH,PORTH_DCMI_PWR_EN,1); // turn on camera
}

void power_up_camera(){
  pin_set_output_state(DCMI_PWR_IOBANK,DCMI_PWR_PIN,1);
}

void power_down_camera(){
  pin_set_output_state(DCMI_PWR_IOBANK,DCMI_PWR_PIN,0);
}
