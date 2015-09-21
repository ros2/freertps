// initial minimal program to sanity-check systems
#include <stdio.h>
#include "freertps/periph/led.h"

int main(int argc, char **argv)
{
  led_init();
  int hello_count = 0;
  led_off();
  while (1)
  {
    for (volatile int i = 0; i < 2000000; i++);
    printf("hello, world! %d\r\n", hello_count++);
  }
  return 0;
}

