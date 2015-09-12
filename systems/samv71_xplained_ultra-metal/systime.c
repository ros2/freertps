#include "systime.h"
#include <stdio.h>

void systime_init()
{
  // let's use TC0 for the system timer.
  // we'll use the first counter to divide down to microseconds,
  // and chain the second 16-bit counter into the third counter
  // to result in a 32-bit count of microseconds since power-up.
  PMC->PMC_PCER0 |= (1 << ID_TC0);
  // we want to feed TIMER_CLOCK2 = MCK/8 into timer/counter channel 0
  // we're running the system at 288 MHz, so we'll get 288/2/8 = 18 MHz
  // need to use TCCLKS to select TIMER_CLOCK2 as TCLK0
  // need to select TCLK0 as input to counter 0 in TC0XC0S mux

  // cascade counters into each other
  /*
  TC0->TC_BMR =
    TC_BMR_TC0XC0S_TCLK0 |
    TC_BMR_TC1XC1S_TIOA0 |
    TC_BMR_TC2XC2S_TIOA1 |
    TC_BMR_MAXFILT(63);
    */
   
  // we need WAVSEL = 10 (without trigger) to have RC reset the counter
  // so we can have microseconds streaming out of the first block
  TC0->TC_CHANNEL[0].TC_CMR = 
    TC_CMR_TCCLKS_TIMER_CLOCK2 | // use TIMER_CLOCK2 as input
    TC_CMR_ACPA_SET            |
    TC_CMR_ACPC_CLEAR          |
    TC_CMR_WAVE                | // waveform mode
    TC_CMR_WAVSEL_UP_RC        ; // reset when the counter hits RC
  TC0->TC_CHANNEL[0].TC_RC = 18; // this will reset TC0 every microsecond
  TC0->TC_CHANNEL[0].TC_RA = 9;  // set the output high halfway through
  TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
  // now, feed the output of block 0 into block 1
  TC0->TC_CHANNEL[1].TC_CMR =
    //TC_CMR_TCCLKS_XC1   |
    TC_CMR_TCCLKS_TIMER_CLOCK2   |
    TC_CMR_ACPA_SET     |
    TC_CMR_ACPC_CLEAR   |
    TC_CMR_WAVE         |
    TC_CMR_WAVSEL_UP_RC ;
  TC0->TC_CHANNEL[1].TC_RC = 65535; 
  TC0->TC_CHANNEL[1].TC_RA = 31768; // set output high halfway through
  TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
  //TC0->TC_BCR = TC_BCR_SYNC;
}

uint32_t systime_usecs()
{
  uint32_t t0 = TC0->TC_CHANNEL[0].TC_CV;
  uint32_t t1 = TC0->TC_CHANNEL[1].TC_CV;
  printf("%4d   %8d   %08x  %08x\r\n", (int)t0, (int)t1, 
      (unsigned)TC0->TC_CHANNEL[0].TC_SR,
      (unsigned)TC0->TC_CHANNEL[1].TC_SR);
  //return TC0->TC_CHANNEL[0].TC_CV;
  return 0;
}
