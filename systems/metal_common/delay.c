#include "metal/systime.h"
#include "metal/delay.h"
#include <stdint.h>

// these functions assume that systime is running at microsecond resolution

void delay_ns(uint32_t ns)
{
  // TODO: actually tune this better on an oscilloscope
  for (volatile uint32_t i = 0; i < ns/10; i++) { }
}

void delay_us(uint32_t us)
{
  // todo: care about wraparound
  volatile uint32_t t_start = systime_usecs();
  while (t_start + us > systime_usecs()) { }
}

void delay_ms(uint32_t ms)
{
  // todo: care about wraparound
  volatile uint32_t t_start = systime_usecs();
  while (t_start + 1000 * ms > systime_usecs()) { }
}

