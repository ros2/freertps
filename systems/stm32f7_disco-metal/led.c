#include "freertps/periph/led.h"
#include "pin.h"

#define PORTI_LED 1

void led_init(void)
{
  pin_set_output(GPIOI, PORTI_LED, 0);
}

void led_on(void)
{
  pin_set_output_state(GPIOI, PORTI_LED, 1);
}

void led_off(void)
{
  pin_set_output_state(GPIOI, PORTI_LED, 0);
}

void led_toggle(void)
{
  GPIOI->ODR ^= 1 << PORTI_LED;
}

