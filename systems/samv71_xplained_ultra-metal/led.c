#include "freertps/periph/led.h"
#include "samv71q21.h"

#define PORTA_LED PIO_PA23

void led_init()
{
  PMC->PMC_PCER0 |= (1 << ID_PIOA);
  PIOA->PIO_PER = PIOA->PIO_OER = PIOA->PIO_CODR = PORTA_LED;
}

void led_on()
{
  PIOA->PIO_SODR = PORTA_LED;
}

void led_off()
{
  PIOA->PIO_CODR = PORTA_LED;
}

void led_toggle()
{
  if (PIOA->PIO_PDSR & PORTA_LED)
    led_off();
  else
    led_on();
}

