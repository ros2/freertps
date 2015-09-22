#include "freertps/periph/i2c.h"
#include "pin.h"
#include <stdio.h>

bool i2c_init(void *i2c)
{
  Twihs *twi = (Twihs *)i2c;
  if (twi != TWIHS0)
  {
    printf("ahh unknown TWI peripheral\r\n");
    return false;
  }
  PMC->PMC_PCER0 |= (1 << ID_TWIHS0);
  pin_set_mux(PIOA, 3, PERIPH_A); // sdc0
  pin_set_mux(PIOA, 4, PERIPH_A); // sda0
  // set up 400 kHz i2c timing on our 144 MHz system clock
  //twi->TWIHS_CWGR = TWIHS_CWGR_CHDIV(86) | TWIHS_CWGR_CLDIV(187) | TWIHS_CWGR_CKDIV(2);
  // hmm...this board doesn't have any i2c pullups, so let's do 100 kHz i2c 
  twi->TWIHS_CWGR = 
    TWIHS_CWGR_CHDIV(170) |
    TWIHS_CWGR_CLDIV(170) |
    TWIHS_CWGR_HOLD(0x1f) | // todo: need to solder on physical pullups
    TWIHS_CWGR_CKDIV(2);
  twi->TWIHS_CR = TWIHS_CR_SVDIS; // disable slave mode. ONCE I WAS THE LEARNER
  twi->TWIHS_CR = TWIHS_CR_MSEN;  // enable master mode. NOW I AM THE MASTER
  printf("twi init complete\r\n");

  return true;
}

bool i2c_read(void *i2c, uint8_t device_addr,
    uint8_t reg_addr, uint8_t len, uint8_t *buffer)
{
  Twihs *twi = (Twihs *)i2c;
  if (twi != TWIHS0) // sanity-check... todo: other instances
    return false;
  /*
  printf("starting %d-byte i2c read from device 0x%02x register 0x%02x\r\n",
      (int)len, (unsigned)device_addr, (unsigned)reg_addr);
  */
  twi->TWIHS_MMR =
    TWIHS_MMR_IADRSZ(1) |
    TWIHS_MMR_MREAD     |
    TWIHS_MMR_DADR(device_addr);
  twi->TWIHS_IADR = reg_addr;
  twi->TWIHS_CR = TWIHS_CR_START;
  for (int i = 0; i < len; i++)
  {
    //printf("waiting for byte %d to arrive...\r\n", i);
    while (!(twi->TWIHS_SR & TWIHS_SR_RXRDY));
    buffer[i] = twi->TWIHS_RHR;
    //printf("done! got byte %d\r\n", i);
    if (i == len - 2)
      twi->TWIHS_CR = TWIHS_CR_STOP;
  }
  while (!(twi->TWIHS_SR & TWIHS_SR_TXCOMP));
  //printf("done with twi read\r\n");
  return true;
}

bool i2c_write(void *i2c, uint8_t device_addr,
    uint8_t reg_addr, uint8_t len, uint8_t *buffer)
{
  Twihs *twi = (Twihs *)i2c;
  if (twi != TWIHS0) // sanity-check... todo: other instances
    return false;
  twi->TWIHS_MMR =
    TWIHS_MMR_IADRSZ(1) |
    TWIHS_MMR_DADR(device_addr);
  twi->TWIHS_IADR = reg_addr;
  //twi->TWIHS_CR = TWIHS_CR_START;
  for (int i = 0; i < len; i++)
  {
    twi->TWIHS_THR = buffer[i];
    while (!(twi->TWIHS_SR & TWIHS_SR_TXRDY));
  }
  twi->TWIHS_CR = TWIHS_CR_STOP;
  while (!(twi->TWIHS_SR & TWIHS_SR_TXCOMP));
  return true;
}
