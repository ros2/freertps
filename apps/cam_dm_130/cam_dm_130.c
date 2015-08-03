#include <stdio.h>
#include "sensors/camera.h"
//#include "freertps/freertps.h"
static int img_count;
void image_cb(){
//  printf("in user defined cb \r\n");
//  camera_take_snapshot(); 
  img_count++;
}


int main(){
  #ifdef CAMERA_WRITE_ADDRESS
    printf("define recognized at least");
  #endif
//  IRQn_Type b;
//  dcmi_data_width_t a;
  int i=0, inc=0;
  img_count =0;
  //camera_resolution_t a;
  printf("\r\n\r\n");
  led_init();
  camera_init(image_cb);
  camera_set_mode(CAMERA_MODE_CONTINUOUS);
//  printf("ISR %d\r\n",I2C1->ISR);
//  uint8_t devId = 0;
//  uint8_t globalbuf[0x7F];

//  camera_set_resolution(CAMERA_RESOLUTION_VGA);
  while(1)
  {
    for(i=0;i<180000000;i++){
    }
//    printf("%X,\r\n",DCMI->DR);
    printf("count: %d",img_count);
    img_count = 0;
    led_toggle();
//      camera_write_register(0x12,0x80);
//    camera_set_resolution(1);
//    break;
//      camera_reset();
  }
}
