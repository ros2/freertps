#include "freertps/periph/led.h"
#include "pin.h"

#define PORTG_LED 6

void led_init(void)
{
  pin_set_output(GPIOG, PORTG_LED, 0);
}

void led_on(void)
{
  pin_set_output_state(GPIOG, PORTG_LED, 1);
}

void led_off(void)
{
  pin_set_output_state(GPIOG, PORTG_LED, 0);
}

void led_toggle(void)
{
  GPIOG->ODR ^= 1 << PORTG_LED;
}

