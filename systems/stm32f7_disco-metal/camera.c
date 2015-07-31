#include "camera.h"
#include <stdio.h>
// IRQ vector DMA2CH1 pos54/prio64 address 0x0000 0124 IRQn->DMA2_Stream1_IRQn
// DCMI vector pos78/prio85 address 0x0000 0178 IRQn->DCMI_IRQn
uint32_t camera_read_register(uint32_t regAddr);
void camera_write_register(uint32_t regAddr, uint32_t value);

void camera_init(){
  printf("camera_init()\r\n");
  camera_init_pins();

  i2c_init();
  camera_reset_regs();

  dcmi_init();

  printf("Camera init done\r\n");
  
}
void camera_start(){
  camera_init();
  power_up_camera();
  // set default framerate, colormode, resolution, continuousmode...
}

void camera_set_framerate(){
}
void camera_set_color(){
}
void camera_set_resolution(){
}
void camera_set_mode(camera_mode_t mode){
  uint32_t regAddr, value;
  camera_write_register(regAddr, value);
}
void camera_write_register(uint32_t regAddr, uint32_t value){

}
uint32_t camera_read_register(uint32_t regAddr){
    // check if register exists on camera
    if(camera_registers(regAddr) == NULL){
        printf("register %d doesnt exist on this camera",regAddr)
    }
    i2c_request_data(DeviceAddr, RegAddr);
  return 0;
}

void camera_reset_regs(){
    
}
