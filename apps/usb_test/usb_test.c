#include <stdio.h>
#include "metal/systime.h"
#include "metal/usb.h"
#include "freertps/periph/led.h"

#define TX_INTERVAL 1000000

int main(int argc, char **argv)
{
  usb_init();
  __enable_irq();
  uint32_t last_tx_time = 0;
  while (1)
  {
    uint32_t t = systime_usecs();
    if (t - last_tx_time > TX_INTERVAL)
    {
      last_tx_time = t;
      led_toggle();
    }
  }
  return 0;
}
