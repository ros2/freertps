#include "camera.h"
#include <stdio.h>

//#define DEBUG 

#define CAMERA_RESOLUTION_QVGA_RGB565 3
//#define CAMERA_ADDRESS 0x30
#define CAMERA_ID 0x96
// IRQ vector DMA2CH1 pos54/prio64 address 0x0000 0124 IRQn->DMA2_Stream1_IRQn
// DCMI vector pos78/prio85 address 0x0000 0178 IRQn->DCMI_IRQn
uint8_t _camera_read_register(uint32_t regAddr);
void _camera_write_register(uint32_t regAddr, uint8_t value);

camera_mode_t g_mode;
camera_resolution_t g_resolution;

void camera_init(image_cb_t cb){
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
  dcmi_init(cb);
  printf("Camera init done\r\n");
//  #endif
}

void camera_reset(){
  _camera_write_register(0x12,0x80);
  camera_set_resolution(CAMERA_RESOLUTION_VGA);
  camera_set_mode(CAMERA_MODE_CONTINUOUS);
}
void camera_set_framerate(){
}
void camera_set_color(){
}
//void camera_set_resolution(uint8_t resolution){
void camera_set_resolution(camera_resolution_t resolution){
  uint16_t nbRegs;
  switch(resolution){
    case CAMERA_RESOLUTION_VGA:
      nbRegs = sizeof(Camera_VGA_Config)/sizeof(Camera_VGA_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
        _camera_write_register(Camera_VGA_Config[i][0],Camera_VGA_Config[i][1]);
      }
//      for(uint16_t i = 0; i<nbRegs;i++){
//        printf("%X,",_camera_read_register(Camera_VGA_Config[i][0]));
//      }
      break;
    case CAMERA_RESOLUTION_QVGA:
      nbRegs = sizeof(Camera_QVGA_Config)/sizeof(Camera_QVGA_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
        _camera_write_register(Camera_QVGA_Config[i][0],Camera_QVGA_Config[i][1]);
      }
//      for(uint16_t i = 0; i<nbRegs;i++){
//        printf("%X,",_camera_read_register(Camera_QVGA_Config[i][0]));
//      }
      break;
    case CAMERA_RESOLUTION_QQVGA:
      nbRegs = sizeof(Camera_QQVGA_Config)/sizeof(Camera_QQVGA_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
        _camera_write_register(Camera_QQVGA_Config[i][0],Camera_QQVGA_Config[i][1]);
      }
//      for(uint16_t i = 0; i<nbRegs;i++){
//        printf("%X,",_camera_read_register(Camera_QQVGA_Config[i][0]));
//      }
      break;
    case CAMERA_RESOLUTION_QVGA_RGB565:
      printf("setting resolution\r\n");
      nbRegs = sizeof(Camera_QVGA_RGB565_Config)/sizeof(Camera_QVGA_RGB565_Config[0]);
      for(uint16_t i=0;i<nbRegs;i++){
        _camera_write_register(Camera_QVGA_RGB565_Config[i][0],Camera_QVGA_RGB565_Config[i][1]);
      }
//      for(uint16_t i = 0; i<nbRegs;i++){
//        printf("%X,",_camera_read_register(Camera_QVGA_RGB565_Config[i][0]));
//      }
      g_resolution = resolution;
      delay_ms(5000);
      break;


  }
  printf("\r\n\r\n");
}

void camera_take_snapshot(){
  if(g_mode== CAMERA_MODE_CONTINUOUS)
    camera_set_mode(CAMERA_MODE_SNAPSHOT);
  else
      DCMI->CR |= DCMI_CR_CAPTURE;
}

void camera_set_mode(camera_mode_t mode){
  switch(mode){
    case CAMERA_MODE_SNAPSHOT:
      DCMI->CR &= ~DCMI_CR_CAPTURE;
      DCMI->CR |=  DCMI_CR_CM   ;           // Set snapshot mode  
      DCMI->CR |= DCMI_CR_CAPTURE;
      printf("changing mode to snapshot");
      delay_ms(1000);
      break;

    case CAMERA_MODE_CONTINUOUS:
      DCMI->CR &= ~DCMI_CR_CAPTURE;
      DCMI->CR &= ~DCMI_CR_CM   ;           // Set continuous mode
      DCMI->CR |= DCMI_CR_CAPTURE;
      printf("changing mode to continuous");
      delay_ms(1000);
      break;
  }
  g_mode = mode;
}

void _camera_write_register(uint32_t regAddr, uint8_t value){
//  printf("camera_write_register()\r\n");
//  printf("writing %X at address %X\r\n",value,regAddr);
  i2c_write(I2C1, (uint8_t)CAMERA_WRITE_ADDRESS, regAddr, 1, &value);// register are all 8bits on this camera
  delay_ms(10);
}


uint8_t _camera_read_register(uint32_t regAddr){
  static uint8_t* buffer;
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
//  res = i2c_read(I2C1,((uint16_t)CAMERA_READ_ADDRESS), regAddr,1); //
  i2c_read(I2C1,((uint16_t)CAMERA_READ_ADDRESS), regAddr,1,buffer); //

  return res;
}

