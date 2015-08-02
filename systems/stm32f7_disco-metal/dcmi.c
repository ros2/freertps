#include "dcmi.h"
// This file implements init function for dcmi and dma mo,dules to talk to a camera

// buffsize = 1/2 frame, if resolution VGA: 640*480/2 = 0x2580
#define BUFFER_SIZE 0x2850
static uint32_t aDST_Buffer[BUFFER_SIZE];

// Assume VGA for everything

void dma_init();

void dcmi_init(){
  /************************
   ** DCMI CONFIGURATION***
   ************************/
  printf("dcmi_init()\r\n");
  RCC->AHB2ENR |= RCC_AHB2ENR_DCMIEN;// Enable DCMI clock
  RCC->AHB2RSTR |= RCC_AHB2RSTR_DCMIRST; // Reset DCMI
  // wait ?
  RCC->AHB2RSTR &= ~RCC_AHB2RSTR_DCMIRST; // Release Reset DCMI
  // wait ?

  DCMI->CR &= ~DCMI_CR_EDM_0;           // Setting 8-bit mode
  DCMI->CR &= ~DCMI_CR_EDM_1;           // Idem
  DCMI->CR |=  DCMI_CR_CM   ;           // Set snapshot mode
  DCMI->CR &= ~DCMI_CR_LSM  &           // buffer all receive lines
              ~DCMI_CR_BSM  &           // buffer all receives bytes
              ~DCMI_CR_JPEG &           // receive uncompressed video
              ~DCMI_CR_CROP ;           // receive full image

// Camera config from st OV9655driver
// HSYNC active low, VSYNC active high, PIXCLK sampling en rising edge
  DCMI->CR &= ~DCMI_CR_HSPOL;           // HSYNC active low Official
//  DCMI->CR |= DCMI_CR_HSPOL;           // HSYNC active high
  DCMI->CR |= DCMI_CR_VSPOL;            // VSYNC active high Official
//  DCMI->CR &= ~DCMI_CR_VSPOL;            // VSYNC active low
  DCMI->CR |= DCMI_CR_PCKPOL;           // PIXCLK sampling en rising edge 
  

  DCMI->CR &= ~DCMI_CR_ESS;             // Hardware synchronisation 
//  //Enable interrupts
  DCMI->IER |=  DCMI_IER_FRAME_IE   |     //Frame interrupt
                DCMI_IER_OVF_IE     |     //Overflow interrupt
                DCMI_IER_ERR_IE     |     //Error interrupt
                DCMI_IER_VSYNC_IE   |     //VSYNC interrupt
                DCMI_IER_LINE_IE    ;     //Line interrupt

  NVIC_SetPriority(DCMI_IRQn,6);
  NVIC_EnableIRQ(DCMI_IRQn);

  // finaly enable DCMI
  DCMI->CR |= DCMI_CR_ENABLE;
  while(!(DCMI->CR & DCMI_CR_ENABLE));

  dma_init();
  DCMI->CR |= DCMI_CR_CAPTURE;
}

void dcmi_take_snapshot(){
}

void dma_init(){

//uint32_t Length = 0x2850;
  /***********************
   ** DMA CONFIGURATION***
   ***********************/
//  DCMI_DR address (uint32_t)(DCMI_BASE + 0x28);
RCC->AHB1ENR  |= RCC_AHB1ENR_DMA2EN   ;  // enable DMA2 clock
RCC->AHB1RSTR |= RCC_AHB1RSTR_DMA2RST ;  // reset DMA2 controller
// wait ?
RCC->AHB1RSTR &= ~RCC_AHB1RSTR_DMA2RST ;  // Release DMA2 controller

  //configure DMA2 (Stream1, Channel 1)
  DMA2_Stream1->CR &= ~DMA_SxCR_EN      ;
  while(DMA2_Stream1->CR & DMA_SxCR_EN) ;  // be sure that DMA disabled
  DMA2->LIFCR |=       DMA_LIFCR_CTCIF1 |  // Clear all interrupts flags
                       DMA_LIFCR_CHTIF1 | 
                       DMA_LIFCR_CTEIF1 | 
                       DMA_LIFCR_CDMEIF1|
                       DMA_LIFCR_CFEIF1 ; 

  // Set addresses 
  DMA2_Stream1->PAR = (uint32_t)(DCMI_BASE + 0x28); // DCMI DR register address
  //DMA2_Stream1->PAR = DCMI->DR            ;
  DMA2_Stream1->M0AR = aDST_Buffer        ;

  DMA2_Stream1->CR &= ~DMA_SxCR_CHSEL_1 & // Select Channel 1
                      ~DMA_SxCR_CHSEL_2 ; // Idem
  DMA2_Stream1->CR |=  DMA_SxCR_CHSEL_0 | // Idem
                       //DMA_SxCR_DBM     | // Double buffer mode
                       DMA_SxCR_CIRC    | // Use a circular buffer
                       DMA_SxCR_PL_1    ; // DMA priority High
  DMA2_Stream1->CR &= ~DMA_SxCR_PL_0    & // Idem
                      ~DMA_SxCR_DIR     & // Periph to memory mode
                      ~DMA_SxCR_PINC    ; // Always read at same periph adress
                      ~DMA_SxCR_DBM     ;
  DMA2_Stream1->CR |=  DMA_SxCR_MINC    ; // Inc memory adress eachtime
  DMA2_Stream1->CR &= ~DMA_SxCR_PSIZE_0 ; // Use Word size (32-bit )
  DMA2_Stream1->CR |=  DMA_SxCR_PSIZE_1 ; // idem
  DMA2_Stream1->CR &= ~DMA_SxCR_MSIZE_0 ; // Use Word size (32-bit )
  DMA2_Stream1->CR |=  DMA_SxCR_MSIZE_1 ; // idem

  DMA2_Stream1->CR |=   DMA_SxCR_TCIE     ;//| // Enable DMA Interrupts
                        DMA_SxCR_HTIE     | // Idem
                        DMA_SxCR_TEIE     | // Idem
                        DMA_SxCR_DMEIE    ; // Idem
  DMA2_Stream1->NDTR = BUFFER_SIZE        ;
  //Enable IRQs
  NVIC_SetPriority(DMA2_Stream1_IRQn,2);
  NVIC_EnableIRQ(DMA2_Stream1_IRQn);

  DMA2_Stream1->CR|= DMA_SxCR_EN          ; //Finally enable DMA
  while(!(DMA2_Stream1->CR & DMA_SxCR_EN));
}

void dma2_stream1_vector(){
  if((DMA2->LISR & DMA_LISR_TCIF1)!=0){
    DMA2->LIFCR |= DMA_LIFCR_CTCIF1;
    printf("Transmit complete, %X\r\n",(DMA2->LISR & DMA_LISR_TCIF1));
  }
}

void dcmi_vector(){
  if((DCMI->RISR & DCMI_RISR_FRAME_RIS) != 0){
    printf("frame interrupt received\r\n,%X",DCMI->DR);
    DCMI->ICR |= DCMI_ICR_FRAME_ISC;
    for(int i=0; i<BUFFER_SIZE;i++){
      printf("%X,",aDST_Buffer[i]);
    }
    delay_ms(1000);
    DCMI->CR |= DCMI_CR_CAPTURE;
  }
  if((DCMI->RISR & DCMI_RISR_OVF_RIS) != 0){
    printf("overrun interrupt received\r\n");
    DCMI->ICR |= DCMI_ICR_OVF_ISC;
    delay_ms(1000);
//    while(1);
  }
  if((DCMI->RISR & DCMI_RISR_ERR_RIS) != 0){
    printf("Error interrupt received\r\n");
    DCMI->ICR |= DCMI_ICR_ERR_ISC;
  }
  if((DCMI->RISR & DCMI_RISR_VSYNC_RIS) != 0){
//    printf("VSYNC interrupt received,");
//    printf("vsyn, 0x%X,0x%X\r\n",DCMI->DR,aDST_Buffer[0]);
//    printf("vsyn, 0x%X\r\n",DCMI->DR);
    DCMI->ICR |= DCMI_ICR_VSYNC_ISC;
  }
  if((DCMI->RISR & DCMI_RISR_LINE_RIS) != 0){
//    printf("Line interrupt received\r\n");
//    printf("line, 0x%X,0x%X\r\n",DCMI->DR,DCMI->SR);
//    printf("line%X,\r\n",DCMI->DR);
    DCMI->ICR |= DCMI_ICR_LINE_ISC;
  }
}


/* OLD GENERAL CONFIG

  #ifndef DCMI_DATA_WIDTH
    #define DCMI_DATA_WIDTH 8
  #endif
  // Set DCMI data width
//  if(DCMI_DATA_WIDTH == 8){
    // For this specific board only 8bits are routed to the camera connector
    DCMI->CR &= ~DCMI_CR_EDM_0;
    DCMI->CR &= ~DCMI_CR_EDM_1;
//  }

  else if(DCMI_DATA_WIDTH == 10){
      DCMI->CR |=  DCMI_CR_EDM_0;
      DCMI->CR &= ~DCMI_CR_EDM_1;
  }
  else if(DCMI_DATA_WIDTH == 12){
        DCMI->CR &= ~DCMI_CR_EDM_0;
        DCMI->CR |=  DCMI_CR_EDM_1;

  }
  else if(DCMI_DATA_WIDTH == 14){
          DCMI->CR |=  DCMI_CR_EDM_0;
          DCMI->CR |=  DCMI_CR_EDM_1;
  }


////    DCMI->CR &= ~DCMI_CR_CM;              // Set Continuous mode otherwise
//      // Select framerate in continuous mode
////    DCMI->CR |= DCMI_CR_FCRC_0/1 //(divide by 1 to 4)
//


  
//  // Embedded Synchronisation otherwise
//  DCMI->CR |= DCMI_CR_ESS;
//  DCMI->ESCR // set embedded synchronization code
  
*/
