#include "camera.h"
#include <stdio.h>

//#define DEBUG 
#define CAMERA_READ_ADDRESS 0x61
#define CAMERA_WRITE_ADDRESS 0x60
//#define CAMERA_ADDRESS 0x30
#define CAMERA_ID 0x96
// IRQ vector DMA2CH1 pos54/prio64 address 0x0000 0124 IRQn->DMA2_Stream1_IRQn
// DCMI vector pos78/prio85 address 0x0000 0178 IRQn->DCMI_IRQn
uint8_t camera_read_register(uint32_t regAddr, uint8_t* buffer);
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

//  dcmi_init();

  i2c_init(I2C1);
  
  camera_power_up();
  delay_ms(1000);
  camera_reset();
  #ifdef DEBUG
  printf("Camera init done\r\n");
  #endif
}

void camera_reset(){
//  #ifdef DEBUG
//  printf("camera_reset()\r\n");
//  #endif
//  uint8_t devId;
//  camera_read_register(0x12, &devId);                 //
//  camera_write_register(0x12, 0x02);     // 
//  devId=1;
//  camera_read_register(0x12, &devId);                 //
//  printf("%X\r\n",devId);
//  camera_write_register(0x12, 0x82);     // 
//  devId=2;
//  camera_read_register(0x12, &devId);                 //
//  printf("%X\r\n",devId);
  uint8_t *df;
  for(uint8_t i = 0; i<0x7F;i++){
    camera_read_register(i,df);
  }
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
void camera_write_register(uint32_t regAddr, uint8_t value){
//  #ifdef DEBUG
  printf("camera_write_register()\r\n");
//  #endif
    // check if register exists on camera
  uint8_t regValue = 0;
  if(0){//camera_registers(regAddr) == NULL){
  #ifdef DEBUG
      printf("register %d doesnt exist on this camera\r\n",regAddr);
  #endif
  }
  uint8_t *buffer;
//  i2c_read_data(I2C1,((uint8_t)CAMERA_READ_ADDRESS), regAddr,1,buffer);
  buffer[0] = value & 0xFF;
  printf("writing %X at address %X\r\n",buffer[0],regAddr);
  i2c_write_data(I2C1, (uint8_t)CAMERA_WRITE_ADDRESS, regAddr, 1, buffer);// register are all 8bits on this camera
  delay_ms(200);
}


uint8_t camera_read_register(uint32_t regAddr, uint8_t* buffer){
  #ifdef DEBUG
  printf("camera_read_register()\r\n"); 
  #endif
  uint8_t buf = 0;
  buffer[0] = 0;
    // check if register exists on camera
  uint8_t regValue = 0;
  if(0){//camera_registers(regAddr) == NULL){
    #ifdef DEBUG
      printf("register %d doesnt exist on this camera\r\n",regAddr);
    #endif
  }
  // register are all 8bits on this camera
  i2c_read_data(I2C1,((uint16_t)CAMERA_READ_ADDRESS), regAddr,1,&buf); //
//  i2c_read_data(I2C1,((uint16_t)CAMERA_ADDRESS), regAddr,1,buffer); //

  return buf;
}

