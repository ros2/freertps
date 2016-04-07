#include "freertps/periph/led.h"
#include "pin.h"

// let's use LED4, since it's green, and green means that everything is OK
#define LED_PIN 12
#define LED_GPIO GPIOD

// this board has lots of LEDs though. maybe we should refactor this library
// so that it can take the index of LED to turn on.

void led_init(void)
{
  pin_set_output(LED_GPIO, LED_PIN, 0);
}

void led_on(void)
{
  pin_set_output_state(LED_GPIO, LED_PIN, 1);
}

void led_off(void)
{
  pin_set_output_state(LED_GPIO, LED_PIN, 0);
}

void led_toggle(void)
{
  LED_GPIO->ODR ^= 1 << LED_PIN;
}

