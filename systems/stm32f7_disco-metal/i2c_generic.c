#include "i2c.h"
#define APB_MHZ 42

// assuming rising time 700n and falling time 100n
// if clk is 42MHz
  
// if clk == 50MHz


void i2c_init(I2C_TypeDef *i2c){

  //#ifdef DEBUG
  printf("i2c_init()\r\n");
  //#endif
//  /***********************
//   ** I2C CONFIGURATION***
//   ***********************/
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


void i2c_write_data(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte, uint8_t* data){
  //#ifdef DEBUG
  //printf("I2C_write_data() %X at address %X\r\n",data[0],RegAddr);
  //#endif
  while((i2c->ISR & I2C_ISR_BUSY)!= 0);          // check I2C bus status
  //#ifdef DEBUG
  //printf("I2C_wrtie_data() CR2:%X\r\n",i2c->CR2);
  //#endif
  while((i2c->CR2 & I2C_CR2_START)!=0){
  //#ifdef DEBUG
  //printf("%X\r\n",i2c->CR2); delay_ms(100);
  //#endif
  }

  uint32_t temp=0;
//  i2c->CR2 |= I2C_CR2_STOP;
  i2c->CR2 &= ~I2C_CR2_START;
//  i2c->CR2 &= ~I2C_CR2_STOP;
  

  temp |= ((RegSizeByte+1) << 16) & I2C_CR2_NBYTES ;       // Set NBYTES to write  RegSizeByte + 1 for register address
  temp |= (DeviceAddr & I2C_CR2_SADD);

  temp &= ~I2C_CR2_RD_WRN      ;       // Request Write
  temp |= I2C_CR2_AUTOEND      ;       // Enable AutoEnd
  temp |= I2C_CR2_START        ;       // pull start bit high
  temp = (DeviceAddr & I2C_CR2_SADD) | (RegSizeByte+1) << 16 | I2C_CR2_START;
  i2c->CR2 = temp;
  //#ifdef DEBUG
//  printf("waiting to receive TX successful interrupt\r\n");
  //#endif
  while(!(i2c->ISR & I2C_ISR_TXIS));       // wait transmit successful
  //#ifdef DEBUG
//  printf("Received TXIS\r\n");
//  printf("RegSizeByte : %d, STOPF:%X\r\n",RegSizeByte,i2c->ISR & I2C_ISR_STOPF);
  //#endif
  
  printf("%X",data[0]);
  // send the data
  for(uint16_t i=0;i<RegSizeByte+1;i++){
    if(i==0){
      i2c->TXDR = RegAddr & I2C_TXDR_TXDATA;
    //#ifdef DEBUG
      printf("sending address to write on: %X\r\n",RegAddr);
    //#endif
    }
    else{
    //#ifdef DEBUG
      printf("sending byte %d, value to write %X\r\n",i-1,data[i-1]);
    //#endif
      i2c->TXDR = data[i-1] & I2C_TXDR_TXDATA;
    }
    //#ifdef DEBUG
//    printf("waiting to receive TX successful interrupt, STOPF: %X \r\n",i2c->ISR & I2C_ISR_STOPF);
//    printf("STOPF: %X \r\n",i2c->ISR & I2C_ISR_STOPF);
    //#endif
    while(!(i2c->ISR & I2C_ISR_TXIS)){
    //#ifdef DEBUG
//      printf("ISR:%X CR2:%X\r\n",i2c->ISR,i2c->CR2); //delay_ms(100);
    //#endif
      if((i2c->ISR & I2C_ISR_TC) != 0){
        i2c->CR2 &= ~I2C_CR2_START;
        i2c->CR2 |=  I2C_CR2_STOP;
        break;
      }
    }       // wait transmit successful

  } 
    //#ifdef DEBUG
//  printf("tansmission successful ISR:0x%X, CR2:0x%X\r\n",i2c->ISR,i2c->CR2);
    //#endif
}

void i2c_read_data(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte, uint8_t* buffer){
    //#ifdef DEBUG
  uint8_t* tmpbuf;
//  printf("i2c_read_data()\r\n");
    //#endif
  
  // load txdr

  //Enable interrupts
  uint32_t temp=0;
  //wait until bus idle
  //#ifdef DEBUG
//  printf("waiting bus Idle%X\r\n",i2c->ISR);
  //#endif
  while((i2c->ISR & I2C_ISR_BUSY)!= 0);          // check I2C bus status
  //#ifdef DEBUG
//  printf("waiting start pull down%X\r\n",i2c->ISR);
  //#endif
  while((i2c->CR2 & I2C_CR2_START)!=0);

  // Adress slave
  temp = (DeviceAddr & I2C_CR2_SADD) | 1 << 16  | I2C_CR2_START;
  
  i2c->CR2 = temp;
  while((i2c->ISR & I2C_ISR_TXIS) == 0){
  //#ifdef DEBUG
//    printf("%X\r\n",i2c->ISR); delay_ms(100);
  //#endif
  }
  i2c->TXDR = RegAddr;
  while((i2c->CR2 & I2C_CR2_START) == I2C_CR2_START);
  //Request data
  temp = (DeviceAddr & I2C_CR2_SADD) | ((RegSizeByte << 16) & I2C_CR2_NBYTES) | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START;
  i2c->CR2 = temp;
  for(int rcvByte=0;rcvByte<RegSizeByte;rcvByte++){
  //#ifdef DEBUG
//    printf("waiting for byte %d\r\n",rcvByte);
  //#endif
    while((i2c->ISR & I2C_ISR_RXNE)==0){
  //#ifdef DEBUG
//    printf("waiting for new byte, %X\r\n",i2c->ISR); delay_ms(100);
  //#endif
    }
    //buffer[rcvByte] = i2c->RXDR;
    tmpbuf[rcvByte] = i2c->RXDR;
  //#ifdef DEBUG
//    printf("received a new byte! {%X,%X}\r\n",RegAddr, tmpbuf[rcvByte]);
    printf("{%X,%X},",RegAddr, tmpbuf[rcvByte]);
  //#endif
  }
  buffer = tmpbuf;
}

  //TODO call NVIC functions according to I2C_Typedef provided
void I2C1_ev_vector(){
  printf("\r\nI2C1_ev_vector %X\r\n",I2C1->ISR);
  if(I2C1->ISR & I2C_ISR_RXNE){
    printf("received new data byte: %X\r\n",I2C1->RXDR);
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

void I2C1_er_vector(){
  printf("I2C1_er_vector\r\n");
  printf("ISR %X\r\n",I2C1->ISR);
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
