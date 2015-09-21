// initial minimal program to sanity-check systems
#include "freertps/periph/led.h"

int main(int argc, char **argv)
{
  led_init();
  while (1)
  {
    for (volatile int i = 0; i < 2000000; i++);
    led_toggle();
  }
  return 0;
}

