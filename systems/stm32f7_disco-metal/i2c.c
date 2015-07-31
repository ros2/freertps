#include "i2c.h"

void i2c_init(){

//  /***********************
//   ** I2C CONFIGURATION***
//   ***********************/
  RCC->APB1ENR  |= RCC_APB1ENR_I2C1EN;
  RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;    // Reset I2C1
  // wait ? 
  RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;   // Releasde reset

  I2C1->CR1  &= ~I2C_CR1_PE;                // Make sure peripheral is disable
  
  I2C1->CR1 |=  I2C_CR1_GCEN        |
                I2C_CR1_NOSTRETCH   ;
                
  I2C1->TIMINGR =((uint32_t)0x40912732) ;   //Magic value provided by ST
  I2C1->OAR1 = I2C_OAR1_OA1EN           ;   // Leave default
  
  
  I2C1->CR1  |= I2C_CR1_PE;                 // Enable peripheral

  // Read camera ID: DeviceAddr, IDRegisterAddr
  // Set slave address NBYTES 
  I2C1->CR2 = 0                     ;       // Reset register value
  I2C1->CR2 &=  ~I2C_CR2_AUTOEND    &
                ~I2C_CR2_RELOAD     ;
//                ~I2C
  I2C1->CR2 |= 1 << 16              ;       // 1 Byte to read
  
                                            // Camera I2C address ((uint16_t)0x60)
                                            // Camera ID register Address    0x0A
  // Read camera Address and Vendor ID for I2C communication
  i2c_request_data(((uint16_t)0x60), ((uint16_t)0x0A));
}

void i2c_request_data(uint16_t DeviceAddr, uint16_t RegAddr){
  // check I2C bus status
  // load
}


