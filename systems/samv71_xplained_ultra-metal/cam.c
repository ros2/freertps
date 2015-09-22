#include <stdio.h>
#include "freertps/periph/cam.h"
#include "freertps/periph/i2c.h"
#include "pin.h"
#include "metal/delay.h"

// my bench-setup board is hard-wired to a MT9V034 board hard-wired to i2c 0x58
#define CAM_I2C_ADDR 0x58

#define MT9V034_REG_VER          0x00
#define MT9V034_REG_COL_START    0x01
#define MT9V034_REG_ROW_START    0x02
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
  uint8_t reg_val_swapped[2] = { reg_val & 0xff, (reg_val >> 8) & 0xff };
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
  delay_ms(1);
  // transfer registers with soft-reset
  cam_write_reg(MT9V034_REG_RESET, 1);
  delay_ms(1);
  cam_write_reg(MT9V034_REG_RESET, 0);
  delay_ms(1);
  cam_print_reg(MT9V034_REG_ROW_START);
  cam_print_reg(MT9V034_REG_CONTROL);
  cam_print_reg(MT9V034_REG_TEST_PATTERN);
  cam_print_reg(MT9V034_REG_LOCK);
}
