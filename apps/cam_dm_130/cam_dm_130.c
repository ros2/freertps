#include <stdio.h>
#include "freertps/freertps.h"

int main(){

  int i=0;
  led_init();
  camera_init();
  while(1)
  {
    for(i=0;i<100000000;i++){
    }
      printf("toggling led\r\n");
//      printf("%d\r\n",DCMI->DR);
//      printf("%d\r\n",I2C1->RXDR);
    led_toggle();
  }
}
