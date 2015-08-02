#include <stdio.h>
//#include "freertps/freertps.h"
//#include "camera.h"

//typedef enum{CAMERA_RESOLUTION_VGA,CAMERA_RESOLUTION_QVGA,CAMERA_RESOLUTION_QQVGA} camera_resolution_t;
//typedef enum{CAMERA_MODE_SNAPSHOT,CAMERA_MODE_CONTINUOUS} camera_mode_t;
//typedef enum{DATA_8_BITS,DATA_10_BITS,DATA_12_BITS,DATA_14_BITS} dcmi_data_width_t;

int main(){
  #ifdef CAMERA_WRITE_ADDRESS
    printf("define recognized at least");
  #endif
//  IRQn_Type b;
//  dcmi_data_width_t a;
  int i=0, inc=0;
//  camera_resolution_t a;
  printf("\r\n\r\n");
  led_init();
  camera_init();

//  printf("ISR %d\r\n",I2C1->ISR);
//  uint8_t devId = 0;
//  uint8_t globalbuf[0x7F];

  while(1)
  {
    for(i=0;i<100000000;i++){
    }
//    printf("%X,\r\n",DCMI->DR);
    led_toggle();
//      camera_write_register(0x12,0x80);
//    camera_set_resolution(1);
//    break;
//      camera_reset();
  }
}
