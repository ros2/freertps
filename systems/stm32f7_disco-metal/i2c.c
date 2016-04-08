#include "i2c.h"
#include <stdio.h>

// assuming rising time 700n and falling time 100n

void i2c_init(I2C_TypeDef *i2c){

  //#ifdef DEBUG
  printf("i2c_init()\r\n");
  //#endif
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB1ENR  |= RCC_APB1ENR_I2C1EN;
  RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;    // Reset i2c
  // wait ? 
  RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;   // Release reset

  i2c->CR1  &= ~I2C_CR1_PE;                // Make sure peripheral is disable
  i2c->TIMINGR = (((uint32_t)0x40912732) & ((uint32_t)0xF0FFFFFF)); //value provided by ST: 700ns rise, 100ns fall, 100kHz

  i2c->OAR1 &= ~I2C_OAR1_OA1EN         ;   // Disable OAR1  

  i2c->CR2 &= ~I2C_CR2_ADD10           ;   // 7bits addressing mode
  i2c->CR2 |=  I2C_CR2_AUTOEND| I2C_CR2_NACK;  
               
  i2c->CR1  |= I2C_CR1_PE;                 // Enable peripheral
  //#ifdef DEBUG
  //printf("i2c_init_done\r\n");
  //printf("ISR %X\r\n",i2c->ISR);
  //#endif
  //TODO call NVIC functions according to I2C_Typedef provided
  NVIC_SetPriority(I2C1_EV_IRQn,8);       //
  NVIC_EnableIRQ(I2C1_EV_IRQn);           // Enabling Event Interrupts
  NVIC_SetPriority(I2C1_ER_IRQn,8);
  NVIC_EnableIRQ(I2C1_ER_IRQn);           // Enabling Error Interrupts
}


void i2c_write(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte, uint8_t* data){
  while((i2c->ISR & I2C_ISR_BUSY)!= 0);          // check I2C bus status
  while((i2c->CR2 & I2C_CR2_START)!=0){
  }

  uint32_t temp=0;
  i2c->CR2 &= ~I2C_CR2_START;
  
  temp |= ((RegSizeByte+1) << 16) & I2C_CR2_NBYTES ;       // Set NBYTES to write  RegSizeByte + 1 for register address
  temp |= (DeviceAddr & I2C_CR2_SADD);

  temp &= ~I2C_CR2_RD_WRN      ;       // Request Write
  temp |= I2C_CR2_AUTOEND      ;       // Enable AutoEnd
  temp |= I2C_CR2_START        ;       // pull start bit high
  temp = (DeviceAddr & I2C_CR2_SADD) | (RegSizeByte+1) << 16 | I2C_CR2_START;
  i2c->CR2 = temp;
  while(!(i2c->ISR & I2C_ISR_TXIS));       // wait transmit successful
  
//  printf("%X",data);
  // send the data
  for(uint16_t i=0;i<RegSizeByte+1;i++){
    if(i==0){
      i2c->TXDR = RegAddr & I2C_TXDR_TXDATA;
    //#ifdef DEBUG
//      printf("sending address to write on: %X\r\n",RegAddr);
    //#endif
    }
    else{
    //#ifdef DEBUG
//      printf("sending byte %d, value to write %X\r\n",i-1,data[i-1]);
    //#endif
      i2c->TXDR = data[i-1] & I2C_TXDR_TXDATA;
    }
    while(!(i2c->ISR & I2C_ISR_TXIS)){              // if TX successful interrupt, end transmission and exit function
      if((i2c->ISR & I2C_ISR_TC) != 0){             // if Transmission Complete interrupt, exit function
        i2c->CR2 &= ~I2C_CR2_START;
        i2c->CR2 |=  I2C_CR2_STOP;
        break;
      }
    }       // wait transmit successful
  }
}

//uint8_t i2c_read_byte(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte){
void i2c_read(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte,uint8_t* buffer){
//  uint8_t res=0;

  //Enable interrupts
  uint32_t temp=0;
  while((i2c->ISR & I2C_ISR_BUSY)!= 0);          // check I2C bus status
  while((i2c->CR2 & I2C_CR2_START)!=0);

  // Adress slave
  temp = (DeviceAddr & I2C_CR2_SADD) | 1 << 16  | I2C_CR2_START;
  
  i2c->CR2 = temp;
  while((i2c->ISR & I2C_ISR_TXIS) == 0){
  }
  i2c->TXDR = RegAddr;
  while((i2c->CR2 & I2C_CR2_START) == I2C_CR2_START);
  //Request data
  temp = (DeviceAddr & I2C_CR2_SADD) | ((RegSizeByte << 16) & I2C_CR2_NBYTES) | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START;
  i2c->CR2 = temp;
  for(int rcvByte=0;rcvByte<RegSizeByte;rcvByte++){
    while((i2c->ISR & I2C_ISR_RXNE)==0){
    }
//    res = i2c->RXDR & 0xFF;
    buffer[rcvByte] = i2c->RXDR & 0xFF;
//    printf("{%X,%X},",RegAddr, res);
  }
//  return res;
}

  //TODO call NVIC functions according to I2C_Typedef provided
void I2C1_ev_vector(void){
  printf("\r\nI2C1_ev_vector %X\r\n", (unsigned)I2C1->ISR);
  if(I2C1->ISR & I2C_ISR_RXNE){
    printf("received new data byte: %X\r\n", (unsigned)I2C1->RXDR);
  }
  else if(I2C1->ISR & I2C_ISR_TC){
    printf("transmission complete !\r\n");
  }
  else if(I2C1->ISR & I2C_ISR_NACKF){
    printf("received NACK signal\r\n");
    I2C1->ICR |= I2C_ICR_NACKCF;
  }
  else if(I2C1->ISR & I2C_ISR_STOPF){
    printf("received STOP signal\r\n");
    I2C1->ICR |= I2C_ICR_STOPCF ;
  }
}

void I2C1_er_vector(void){
  printf("I2C1_er_vector\r\n");
  printf("ISR %X\r\n", (unsigned)I2C1->ISR);
}

/*Interrupt list */
//  i2c->CR1 |= I2C_CR1_TXIE             |     // Enable interrupts
//              I2C_CR1_RXIE             |
//              I2C_CR1_ADDRIE           |
//              I2C_CR1_NACKIE           |
//              I2C_CR1_STOPIE           |
//              I2C_CR1_TCIE             |
//              I2C_CR1_ERRIE            ;

//  i2c->ICR |= I2C_ICR_ADDRCF       | 
//              I2C_ICR_NACKCF        |
//              I2C_ICR_STOPCF        | 
//              I2C_ICR_BERRCF        |
//              I2C_ICR_ARLOCF        | 
//              I2C_ICR_OVRCF         |
//              I2C_ICR_PECCF         |
//              I2C_ICR_TIMOUTCF      |
//              I2C_ICR_ALERTCF       ;
