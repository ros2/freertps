#include "camera.h"
#include <stdio.h>
#include "delay.h"

//FIXME this code is highly correlated to the OV9655 camera. 
//The registers are valids only for this specific chip
//#define DEBUG 
uint8_t _camera_read_register(uint32_t regAddr);
void _camera_write_register(uint32_t regAddr, uint8_t value);
camera_config_t g_camera_config;

//TODO implement
void camera_increase_brightness();
void camera_decrease_brightness();
void camera_increase_contrast();
void camera_decrease_contrast();
//void camera_init(image_cb_t cb){
void camera_init(image_cb_t cb,image_cb_t dma_cb){
  #ifdef DEBUG
  printf("camera_init()\r\n");
  #endif
  camera_init_pins();
  camera_power_down();

  i2c_init(I2C1);
  
  camera_power_up();
  delay_ms(500);
  camera_reset();
  camera_set_resolution(CAMERA_RESOLUTION_QVGA);
//  #ifdef DEBUG
//  dcmi_init(cb);
  dcmi_init(cb, dma_cb);
  printf("Camera init done\r\n");
//  #endif
}

void camera_reset(){
  _camera_write_register(0x12,0x80);
  camera_set_resolution(CAMERA_RESOLUTION_VGA);
  camera_set_mode(CAMERA_MODE_SNAPSHOT);
  camera_set_framerate(CAMERA_FRAMERATE_30FPS);
}

void camera_set_framerate(camera_framerate_t framerate){
  if(framerate == CAMERA_FRAMERATE_15FPS){
    _camera_write_register(0x12,(_camera_read_register(0x12) & 0x1F));
  }
  if(framerate == CAMERA_FRAMERATE_30FPS){
    _camera_write_register(0x12,(_camera_read_register(0x12) & 0x1F) | 0x60);
  }
  g_camera_config.framerate = framerate;
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
  }
  g_camera_config.resolution = resolution;
//  delay_ms(5000);
//  printf("\r\n\r\n");
}

void camera_take_snapshot(){
  if(g_camera_config.mode== CAMERA_MODE_CONTINUOUS)
    camera_set_mode(CAMERA_MODE_SNAPSHOT);
  else
      DCMI->CR |= DCMI_CR_CAPTURE;
}

void camera_set_mode(camera_mode_t mode){
  if(mode ==CAMERA_MODE_SNAPSHOT){
      DCMI->CR &= ~DCMI_CR_CAPTURE;
      DCMI->CR |=  DCMI_CR_CM   ;           // Set snapshot mode  
      DCMI->CR |= DCMI_CR_CAPTURE;
      printf("changing mode to snapshot\r\n");
  }
  if(mode == CAMERA_MODE_CONTINUOUS){
      DCMI->CR &= ~DCMI_CR_CAPTURE;
      DCMI->CR &= ~DCMI_CR_CM   ;           // Set continuous mode
      DCMI->CR |= DCMI_CR_CAPTURE;
      printf("changing mode to continuous\r\n");
  }
      delay_ms(1000);
  g_camera_config.mode = mode;
}

void camera_set_color(camera_colorspace_t color_format)
{
  if(color_format == CAMERA_COLOR_RGB_565){
    _camera_write_register(0x12,(_camera_read_register(0x12) & 0xFC) | 0x03);
    _camera_write_register(0x40,(_camera_read_register(0x40) & 0x0F) | 0xD0);
  }
  if(color_format == CAMERA_COLOR_RGB_555){
    _camera_write_register(0x12,(_camera_read_register(0x12) & 0xFC) | 0x03);
    _camera_write_register(0x40,(_camera_read_register(0x40) & 0x0F) | 0xF0);

  }
  if(color_format == CAMERA_COLOR_YUV){
    _camera_write_register(0x3A,(_camera_read_register(0x3A) & 0xE3) | 0x0C);
  }
  g_camera_config.colorspace = color_format;
}

void camera_set_nightmode(uint8_t mode){
  uint8_t tmp = _camera_read_register(0x3B) & 0x1F; // reset night mode and night mode insert frame
  if(mode == 1){
    tmp|=0x80;
  }
  _camera_write_register(0x3B, tmp);         // enable nightmode with normale framerate
}

//Private functions
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

