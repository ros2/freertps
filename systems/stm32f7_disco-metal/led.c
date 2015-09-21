#include "freertps/periph/led.h"
#include "pin.h"

#define PORTI_LED 1

void led_init()
{
  pin_set_output(GPIOI, PORTI_LED, 0);
}

void led_on()
{
  pin_set_output_state(GPIOI, PORTI_LED, 1);
}

void led_off()
{
  pin_set_output_state(GPIOI, PORTI_LED, 0);
}

void led_toggle()
{
  GPIOI->ODR ^= 1 << PORTI_LED;
}

