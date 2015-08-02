#include <stdio.h>
#include "freertps/freertps.h"

int main(){

  int i=0, inc=0;
  printf("\r\n\r\n");
  led_init();
  camera_init();

  printf("ISR %d\r\n",I2C1->ISR);
  uint8_t devId = 0;
  uint8_t globalbuf[0x7F];
  led_toggle();

  while(inc < 0xC6)
  {
    for(i=0;i<1000000;i++){
    }
      camera_write_register(0x12,0x80);
      camera_reset();
  }
}
