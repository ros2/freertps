#include "freertps/periph/led.h"
#include "samv71q21.h"

#define PORTA_LED PIO_PA23

void led_init(void)
{
  PMC->PMC_PCER0 |= (1 << ID_PIOA);
  PIOA->PIO_PER = PIOA->PIO_OER = PIOA->PIO_CODR = PORTA_LED;
}

void led_on(void)
{
  PIOA->PIO_SODR = PORTA_LED;
}

void led_off(void)
{
  PIOA->PIO_CODR = PORTA_LED;
}

void led_toggle(void)
{
  if (PIOA->PIO_PDSR & PORTA_LED)
    led_off();
  else
    led_on();
}

