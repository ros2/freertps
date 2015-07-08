#include "led.h"
#include "pin.h"
#include "stm32f427xx.h"

#define PORTC_LED 2

void led_init()
{
  pin_set_output(GPIOC, PORTC_LED, 0);
}

void led_on()
{
  pin_set_output_state(GPIOC, PORTC_LED, 1);
}

void led_off()
{
  pin_set_output_state(GPIOC, PORTC_LED, 0);
}

void led_toggle()
{
  GPIOC->ODR ^= 1 << PORTC_LED;
}

