#include "camera.h"
#include <stdio.h>

//#define DEBUG 
#define CAMERA_READ_ADDRESS 0x61
#define CAMERA_WRITE_ADDRESS 0x60

#define CAMERA_RESOLUTION_VGA 0
#define CAMERA_RESOLUTION_QVGA 1
#define CAMERA_RESOLUTION_QQVGA 2
//#define CAMERA_ADDRESS 0x30
#define CAMERA_ID 0x96
// IRQ vector DMA2CH1 pos54/prio64 address 0x0000 0124 IRQn->DMA2_Stream1_IRQn
// DCMI vector pos78/prio85 address 0x0000 0178 IRQn->DCMI_IRQn
uint8_t camera_read_register(uint32_t regAddr);
void camera_write_register(uint32_t regAddr, uint8_t value);
void camera_reset();
// camerta register data size 8bits
void camera_init(){
  #ifdef DEBUG
  printf("camera_init()\r\n");
  #endif
  camera_init_pins();
  camera_power_down();
//  printf("call_i2c_init()\r\n");
//  printf("call camera_reset_regs()\r\n");
//  printf("call camera_reset_regs()");


  i2c_init(I2C1);
  
  camera_power_up();
  delay_ms(500);
  camera_reset();
  camera_set_resolution(0);
//  #ifdef DEBUG
  dcmi_init();
  printf("Camera init done\r\n");
  dcmi_take_snapshot();
//  #endif
}

void camera_reset(){
  uint8_t i = 0;
//  printf("\r\n%d,%X\r\n",(sizeof(Camera_VGA_Config)/sizeof(Camera_VGA_Config[0])),Camera_VGA_Config[0][0]);
//  for(i = 0; i < 0xC7;i++){
//    camera_read_register(i);
//  }
//  printf("\r\n\r\n");
  camera_write_register(0x12,0x80);
//  for(i = 0; i<0xC7;i++){
//    camera_read_register(i);
//  }
//  printf("\r\n\r\n");
//  // add VGA configuration
//  for(i = 0; i<(sizeof(Camera_VGA_Config)/sizeof(Camera_VGA_Config[0]));i++){
//    camera_write_register(Camera_VGA_Config[i][0],Camera_VGA_Config[i][1]);
//  }
//  printf("\r\n\r\n");

}
void camera_set_framerate(){
}
void camera_set_color(){
}
void camera_set_resolution(uint8_t resolution){
  uint16_t nbRegs;
  switch(resolution){
    case CAMERA_RESOLUTION_VGA:
      nbRegs = sizeof(Camera_VGA_Config)/sizeof(Camera_VGA_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
//        printf("{%X,%X},",Camera_VGA_Config[i][0],Camera_VGA_Config[i][1]);
        camera_write_register(Camera_VGA_Config[i][0],Camera_VGA_Config[i][1]);
      }
      for(uint16_t i = 0; i<nbRegs;i++){
        camera_read_register(Camera_VGA_Config[i][0]);
      }
      break;
    case CAMERA_RESOLUTION_QVGA:
      nbRegs = sizeof(Camera_QVGA_Config)/sizeof(Camera_QVGA_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
//        printf("{%X,%X},",Camera_QVGA_Config[i][0],Camera_QVGA_Config[i][1]);
        camera_write_register(Camera_QVGA_Config[i][0],Camera_QVGA_Config[i][1]);
      }
      for(uint16_t i = 0; i<nbRegs;i++){
        camera_read_register(Camera_QVGA_Config[i][0]);
      }
      break;
    case CAMERA_RESOLUTION_QQVGA:
      nbRegs = sizeof(Camera_QQVGA_Config)/sizeof(Camera_QQVGA_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
//        printf("{%X,%X},",Camera_QQVGA_Config[i][0],Camera_QQVGA_Config[i][1]);
        camera_write_register(Camera_QQVGA_Config[i][0],Camera_QQVGA_Config[i][1]);
      }
      for(uint16_t i = 0; i<nbRegs;i++){
        camera_read_register(Camera_QQVGA_Config[i][0]);
      }
      break;
  }
  printf("\r\n\r\n");
}
//void camera_set_mode(camera_mode_t mode){
//  uint32_t regAddr, value;
//  camera_write_register(regAddr, value);
//}
void camera_write_register(uint32_t regAddr, uint8_t value){
//  printf("camera_write_register()\r\n");
//  i2c_read_data(I2C1,((uint8_t)CAMERA_READ_ADDRESS), regAddr,1,buffer);
//  printf("writing %X at address %X\r\n",value,regAddr);
  i2c_write_data(I2C1, (uint8_t)CAMERA_WRITE_ADDRESS, regAddr, 1, value & 0xFF);// register are all 8bits on this camera
  delay_ms(10);
}


uint8_t camera_read_register(uint32_t regAddr){
  #ifdef DEBUG
  printf("camera_read_register()\r\n"); 
  #endif
  uint8_t res = 0;
    // check if register exists on camera
  if(0){//camera_registers(regAddr) == NULL){
    #ifdef DEBUG
      printf("register %d doesnt exist on this camera\r\n",regAddr);
    #endif
  }
  // register are all 8bits on this camera
  res = i2c_read_data(I2C1,((uint16_t)CAMERA_READ_ADDRESS), regAddr,1); //
//  i2c_read_data(I2C1,((uint16_t)CAMERA_ADDRESS), regAddr,1,buffer); //

  return res;
}

