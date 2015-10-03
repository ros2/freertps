#include <stdio.h>
#include "freertps/freertps.h"
#include "freertps/periph/led.h"

void led_cb(const void *msg)
{
  uint8_t led = *((uint8_t *)msg);
  if (led & 0x1)
    led_on();
  else
    led_off();
}

int main(int argc, char **argv)
{
  freertps_system_init();
  freertps_create_sub("led", "std_msgs::msg::dds_::Bool_", led_cb);
  freertps_start();
  while (freertps_system_ok())
  {
    frudp_listen(1000000);
    frudp_disco_tick();
  }
  frudp_fini();
  return 0;
}

