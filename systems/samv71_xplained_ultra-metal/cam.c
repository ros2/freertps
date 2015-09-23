#include <stdio.h>
#include "freertps/periph/cam.h"
#include "freertps/periph/i2c.h"
#include "pin.h"
#include "metal/delay.h"
#include "metal/systime.h"
#include "freertps/periph/led.h"

// my bench-setup board is hard-wired to a MT9V034 board hard-wired to i2c 0x58
#define CAM_I2C_ADDR 0x58

#define MT9V034_REG_VER          0x00
#define MT9V034_REG_COL_START    0x01
#define MT9V034_REG_ROW_START    0x02
#define MT9V034_REG_WIN_HEIGHT   0x03
#define MT9V034_REG_WIN_WIDTH    0x04
#define MT9V034_REG_HORIZ_BLANK  0x05
#define MT9V034_REG_VERT_BLANK   0x06
#define MT9V034_REG_CONTROL      0x07
#define MT9V034_REG_RESET        0x0c
#define MT9V034_REG_TEST_PATTERN 0x7f
#define MT9V034_REG_LOCK         0xfe


static uint16_t cam_read_reg(const uint8_t reg_addr)
{
  uint8_t reg_val[2] = {0};
  if (!i2c_read(TWIHS0, CAM_I2C_ADDR, reg_addr, 2, reg_val))
  {
    printf("error reading register 0x%02x\r\n", reg_addr);
    return 0;
  }
  return (reg_val[0] << 8) | reg_val[1];
}

static bool cam_write_reg(const uint8_t reg_addr, const uint16_t reg_val)
{
  uint8_t reg_val_swapped[2] = { (reg_val >> 8) & 0xff, reg_val & 0xff };
  if (!i2c_write(TWIHS0, CAM_I2C_ADDR, reg_addr, 2, reg_val_swapped))
  {
    printf("error writing register 0x%02x\r\n", reg_addr);
    return false;
  }
  return true;
}

static void cam_print_reg(const uint8_t reg_idx)
{
  const char *reg_name = NULL;
  switch (reg_idx)
  {
    case MT9V034_REG_VER:          reg_name = "version"; break;
    case MT9V034_REG_COL_START:    reg_name = "column start"; break;
    case MT9V034_REG_ROW_START:    reg_name = "row start"; break;
    case MT9V034_REG_CONTROL:      reg_name = "control"; break;
    case MT9V034_REG_RESET:        reg_name = "reset"; break;
    case MT9V034_REG_TEST_PATTERN: reg_name = "test pattern"; break;
    case MT9V034_REG_LOCK:         reg_name = "lock"; break;
    default:                       reg_name = "unknown"; break;
  }
  uint16_t reg_val = cam_read_reg(reg_idx);
  printf("reg 0x%02x (%s) = 0x%04x\r\n", reg_idx, reg_name, reg_val);
}

struct isi_dma_desc_t
{
  uint32_t frame_buffer_addr;
  uint32_t ctrl;
  uint32_t next_desc;
};

static struct isi_dma_desc_t g_isi_dma_desc_t;
uint8_t g_cam_frame_buf[640*360];

void cam_init()
{
  printf("cam_init()\r\n");
  PMC->PMC_PCER0 |= (1 << ID_PIOA) | (1 << ID_PIOB) | (1 << ID_PIOD);
  PMC->PMC_PCER1 |=  1 << (ID_ISI - 32);
  // pin mux assignments as per p.535 of SAM V71 datasheet
  pin_set_mux(PIOD, 22, PERIPH_D); // ISI_D0
  pin_set_mux(PIOD, 21, PERIPH_D); // ISI_D1
  pin_set_mux(PIOB,  3, PERIPH_D); // ISI_D2
  pin_set_mux(PIOA,  9, PERIPH_B); // ISI_D3
  pin_set_mux(PIOA,  5, PERIPH_B); // ISI_D4
  pin_set_mux(PIOD, 11, PERIPH_D); // ISI_D5
  pin_set_mux(PIOD, 12, PERIPH_D); // ISI_D6
  pin_set_mux(PIOA, 27, PERIPH_D); // ISI_D7
  pin_set_mux(PIOD, 24, PERIPH_D); // ISI_HSYNC
  pin_set_mux(PIOA, 24, PERIPH_D); // ISI_PCLK
  pin_set_mux(PIOD, 25, PERIPH_D); // ISI_VSYNC

  pin_enable_pullup(PIOA, 3, true);
  pin_enable_pullup(PIOA, 4, true);

  pin_set_output(PIOB, 13, false); // reset it
  delay_ms(1);
  pin_set_output_state(PIOB, 13, true); // release reset
  delay_ms(1);

  // now, start sending it clock over PCK0
  pin_set_mux(PIOA,  6, PERIPH_B); // enable PCK0 to drive output
  // let's generate a 24 MHz output clock from master clock (144M) / 6
  PMC->PMC_PCK[0] = PMC_PCK_CSS_MCK | PMC_PCK_PRES(5); // divisor is PRES+1
  PMC->PMC_SCER |= PMC_SCER_PCK0;
  while (!(PMC->PMC_SR & PMC_SR_PCKRDY0));
  delay_ms(1); // let camera wake up
  
  if (!i2c_init(TWIHS0))
    printf("couldn't init twi0\r\n");

  cam_print_reg(0);
  /*
  const uint16_t chip_ver = cam_read_reg(0);
  printf("chip ver: 0x%04x\r\n", chip_ver);
  */

  cam_write_reg(MT9V034_REG_RESET, 1);
  delay_ms(1);
  cam_write_reg(MT9V034_REG_RESET, 0);
  delay_ms(1);
  cam_write_reg(MT9V034_REG_CONTROL, 0x0188);
  cam_write_reg(MT9V034_REG_TEST_PATTERN, 0x05);
  cam_write_reg(MT9V034_REG_WIN_WIDTH, 640);
  cam_write_reg(MT9V034_REG_WIN_HEIGHT, 240);
  cam_write_reg(MT9V034_REG_VERT_BLANK, 500);
  delay_ms(1);
  // transfer shadowed registers with soft-reset
  cam_write_reg(MT9V034_REG_RESET, 1);
  delay_ms(1);
  cam_write_reg(MT9V034_REG_RESET, 0);
  delay_ms(1);
  cam_print_reg(MT9V034_REG_ROW_START);
  cam_print_reg(MT9V034_REG_CONTROL);
  cam_print_reg(MT9V034_REG_TEST_PATTERN);
  cam_print_reg(MT9V034_REG_LOCK);

  ISI->ISI_CFG1 = 
    ISI_CFG1_FRATE(7) | // only use every 8th frame
    ISI_CFG1_DISCR; // codec datapath DMA automatically restarts
  // Bayer imager needs the "grayscale" mode of ISI
  ISI->ISI_CFG2 = 
    ISI_CFG2_IM_VSIZE(240) |
    ISI_CFG2_IM_HSIZE(319) ; // needs to be img size / 2

  g_isi_dma_desc_t.frame_buffer_addr = (uint32_t)g_cam_frame_buf;
  g_isi_dma_desc_t.ctrl = 
    ISI_DMA_C_CTRL_C_FETCH |
    ISI_DMA_C_CTRL_C_WB    ; // what is the writeback bit? doc says to use it
  g_isi_dma_desc_t.next_desc = (uint32_t)&g_isi_dma_desc_t; // loop to itself
  __DSB(); // memory sync

  ISI->ISI_DMA_C_ADDR = (uint32_t)&g_isi_dma_desc_t;
  ISI->ISI_DMA_C_CTRL = ISI_DMA_C_CTRL_C_FETCH;
  ISI->ISI_DMA_CHER = ISI_DMA_CHER_C_CH_EN; // enable codec DMA channel
  __DSB(); // memory sync

  __disable_irq();
  NVIC_SetPriority(ISI_IRQn, 4);
  NVIC_EnableIRQ(ISI_IRQn);
  ISI->ISI_CR = ISI_CR_ISI_EN;// | ISI_CR_ISI_CDC; // start your engines
  ISI->ISI_CR = ISI_CR_ISI_CDC;
  ISI->ISI_IER = ISI_IER_CXFR_DONE; // enable interrupt on frame-complete
  __enable_irq();
  printf("isi_sr = %08x\r\n", (unsigned)ISI->ISI_SR);
}

void isi_vector()
{
  printf("isi\r\n");
  //ISI->ISI_DMA_CH
  //ISI->ISI_DMA_C_ADDR = (uint32_t)&g_isi_dma_desc_t;
  ISI->ISI_DMA_CHER = ISI_DMA_CHER_C_CH_EN; // enable codec DMA channel
  volatile uint32_t isi_sr __attribute__((unused)) = ISI->ISI_SR;
  /*
  uint32_t t = systime_usecs();
  static uint32_t t_prev;
  if (t > 2000000)
  {
    //printf("t=%08d frame\r\n", (unsigned)t);
    uint32_t dt = t - t_prev;
    t_prev = t;
    //printf("dt = %08d\r\n", (unsigned)dt);
  }
  */
  led_toggle();
}
