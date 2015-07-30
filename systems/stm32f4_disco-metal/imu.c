#include "sensors/imu.h"
#include <stdio.h>
#include "pin.h"
#include "delay.h"

// stm32f4-disco has an ST LIS3DSH, connected as follows:
// CS   = PE3
// SCLK = PA5 = SPI1 SCLK on AF5
// MOSI = PA7 = SPI1 MOSI on AF5
// MISO = PA6 = SPI1 MISO on AF5

#define PORTE_CS   3
#define PORTA_SCLK 5
#define PORTA_MISO 6
#define PORTA_MOSI 7

static void accel_read_regs(const uint8_t start_reg, 
                            const uint8_t num_regs_to_read,
                            uint8_t *regs);
static uint8_t accel_read_reg(const uint8_t reg);

void imu_init()
{
  printf("imu init\r\n");
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  pin_set_output(GPIOE, PORTE_CS, 1);
  pin_set_alternate_function(GPIOA, PORTA_SCLK, 5);
  pin_set_alternate_function(GPIOA, PORTA_MISO, 5);
  pin_set_alternate_function(GPIOA, PORTA_MOSI, 5);
  pin_set_output_speed(GPIOA, PORTA_SCLK, 3);
  pin_set_output_speed(GPIOA, PORTA_MOSI, 3);
  pin_set_output_speed(GPIOE, PORTE_CS  , 3);
  SPI1->CR1 = SPI_CR1_SSM  | // software slave select management
              SPI_CR1_SSI  | // assert software select state
              SPI_CR1_CPHA | // clock phase: cpha = 1, cpol = 1
              SPI_CR1_CPOL | // ditto
              SPI_CR1_MSTR | // master mode
              SPI_CR1_BR_0 | // baud rate sel. need to calculate.
              SPI_CR1_BR_1 | // ditto
              SPI_CR1_SPE  ; // enable SPI
  delay_us(100);
  uint8_t info1 = accel_read_reg(0x0d);
  printf("info1 = 0x%02x\r\n", (unsigned)info1);
  float test_accel[3] = {0};
  imu_poll_accels(test_accel);
}

static uint8_t accel_read_reg(const uint8_t reg)
{
  uint8_t reg_val = 0;
  accel_read_regs(reg, 1, &reg_val);
  return reg_val;
}

static void accel_read_regs(const uint8_t start_reg, 
                            const uint8_t num_regs_to_read,
                            uint8_t *regs)
{
  pin_set_output_low(GPIOE, PORTE_CS);
  volatile uint8_t __attribute__((unused)) rxd = SPI1->DR; // flush first!
  SPI1->DR = 0x80 | start_reg;
  while (!(SPI1->SR & SPI_SR_RXNE)) { } // wait for it...
  rxd = SPI1->DR; // flush garbage (first inbound byte)
  for (int i = 0; i < num_regs_to_read; i++)
  {
    while (!(SPI1->SR & SPI_SR_TXE)) { }
    SPI1->DR = 0; // start SPI txrx
    while (!(SPI1->SR & SPI_SR_RXNE)) { } // wait for it...
    regs[i] = SPI1->DR; // save the data byte that we received
  }
  while (SPI1->SR & SPI_SR_BSY) { }
  delay_ns(1); // burn a few cycles (TODO: is this needed? scope it sometime.)
  pin_set_output_high(GPIOE, PORTE_CS);
}

bool imu_poll_accels(float *xyz)
{
  uint8_t raw_read[6];
  accel_read_regs(0x28, 6, &raw_read);
  printf("raw read: %02x%02x  %02x%02x  %02x%02x\r\n",
         raw_read[0], raw_read[1],
         raw_read[2], raw_read[3],
         raw_read[4], raw_read[5]);
  int16_t raw_accel[3];
  for (int i = 0; i < 3; i++)
    raw_accel[i] = raw_read[i*2] | ((int16_t)raw_read[i*2+1]);
  printf("raw accel: [%d %d %d]\n",
         raw_accel[0], raw_accel[1], raw_accel[2]);

  // TODO: actually use the hardware
  xyz[0] = 1;
  xyz[1] = 2;
  xyz[2] = 3;
  return true;
}

