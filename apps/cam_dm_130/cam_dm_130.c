#include <stdio.h>
#include "sensors/camera.h"
#include "peripheral/usb.h"
//#include "freertps/freertps.h"
static int img_count;
static int dma_count;

void image_cb(){
//  printf("in user defined cb \r\n");
//  camera_take_snapshot(); 
  img_count++;
//  printf("dma count: %d\r\n",dma_count);
//  dma_count=0;
  
//  printf("%X,",LTDC_Layer1->CLUTWR);
//  printf("%d\r\n",dma_count);
//  dma_count = 0;
}
void dma_cb(){
  dma_count++;
}

int main(){
  #ifdef CAMERA_WRITE_ADDRESS
    printf("define recognized at least");
  #endif
  int i=0, inc=0;
  img_count =0;

  printf("RCC CR:           0x%8X\r\n",RCC->CR);
  printf("RCC CFGR:         0x%8X\r\n",RCC->CFGR);
  printf("RCC PLLCFGR:      0x%8X\r\n",RCC->PLLCFGR);
  printf("RCC_PLLSAICFGR1:  0x%8X\r\n",RCC->PLLSAICFGR);
  printf("RCC_DCKCFGR1:     0x%8X\r\n",RCC->DCKCFGR1);
  printf("RCC_DCKCFGR2:     0x%8X\r\n",RCC->DCKCFGR2);
  printf("RCC->APB2ENR:     0x%8X\r\n",RCC->APB2ENR);

  led_init();
//  lcd_init();
  printf("power settings:\r\n");
  printf("PWR->CR1:         0x%8X\r\n",PWR->CR1);
  printf("PWR->CR2:         0x%8X\r\n",PWR->CR2);
  printf("RCC CR:           0x%8X\r\n",RCC->CR);
  printf("RCC CFGR:         0x%8X\r\n",RCC->CFGR);
  printf("RCC PLLCFGR:      0x%8X\r\n",RCC->PLLCFGR);
  printf("RCC_PLLSAICFGR1:  0x%8X\r\n",RCC->PLLSAICFGR);
  printf("RCC_DCKCFGR1:     0x%8X\r\n",RCC->DCKCFGR1);
  printf("RCC_DCKCFGR2:     0x%8X\r\n",RCC->DCKCFGR2);
  printf("RCC->APB2ENR:     0x%8X\r\n",RCC->APB2ENR);
  printf("RCC->APB1ENR:     0x%8X\r\n",RCC->APB1ENR);
  printf("LTDC->SSCR:       0x%8X\r\n",LTDC->SSCR);
  printf("LTDC->BPCR:       0x%8X\r\n",LTDC->BPCR);
  printf("LTDC->AWCR:       0x%8X\r\n",LTDC->AWCR);
  printf("LTDC->TWCR:       0x%8X\r\n",LTDC->TWCR);
  printf("LTDC->BCCR:       0x%8X\r\n",LTDC->BCCR);
  printf("LTDC->IER:        0x%8X\r\n",LTDC->IER);
  printf("LTDC->GCR:        0x%8X\r\n",LTDC->GCR);
//  camera_init(image_cb);
  printf("LTDC_Layer1->WVPCR :0x%8X\r\n",LTDC_Layer1->WVPCR );
  printf("LTDC_Layer1->PFCR:  0x%8X\r\n",LTDC_Layer1->PFCR  );
  printf("LTDC_Layer1->DCCR:  0x%8X\r\n",LTDC_Layer1->DCCR  );
  printf("LTDC_Layer1->CACR:  0x%8X\r\n",LTDC_Layer1->CACR  );
  printf("LTDC_Layer1->BFCR:  0x%8X\r\n",LTDC_Layer1->BFCR  );
  printf("LTDC_Layer1->CFBAR: 0x%8X\r\n",LTDC_Layer1->CFBAR );
  printf("LTDC_Layer1->CFBLR: 0x%8X\r\n",LTDC_Layer1->CFBLR );
  printf("LTDC_Layer1->CFBLNR:0x%8X\r\n",LTDC_Layer1->CFBLNR);

//  camera_init(image_cb,dma_cb);
//  camera_set_resolution(CAMERA_RESOLUTION_QVGA);
//  camera_set_mode(CAMERA_MODE_CONTINUOUS);
  usbd_fs_init();
  uint8_t testUSB[6]="coucou";
  //usb_tx(1,testUSB,6);
  while(1){
    for(i=0;i<170000000;i++){
    }
//    printf("%X,\r\n",DCMI->DR);
    printf("count: %d,%d\r\n",img_count,dma_count);
    img_count = 0;
    dma_count = 0;
    led_toggle();
//      camera_write_register(0x12,0x80);
//    camera_set_resolution(1);
//    break;
//      camera_reset();
  }
}

