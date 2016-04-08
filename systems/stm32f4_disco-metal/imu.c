#include "freertps/periph/imu.h"
#include <stdio.h>
#include "pin.h"
#include "metal/delay.h"

// stm32f4-disco has an ST LIS3DSH, connected as follows:
// CS   = PE3
// SCLK = PA5 = SPI1 SCLK on AF5
// MOSI = PA7 = SPI1 MOSI on AF5. other option (via cut trace, jumper): PB5
// MISO = PA6 = SPI1 MISO on AF5

#define PORTE_CS   3
#define PORTA_SCLK 5
#define PORTA_MISO 6
#define PORTB_MOSI 5

//#define PRINT_REGISTER_TABLE

static void accel_txrx(const uint8_t start_reg, 
                       const uint8_t num_regs,
                       uint8_t *rx,
                       const uint8_t *tx);
static void accel_write_reg(const uint8_t reg, const uint8_t val); 
#ifdef PRINT_REGISTER_TABLE
static uint8_t accel_read_reg(const uint8_t reg);
#endif

void imu_init(void)
{
  printf("imu init\r\n");
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  pin_set_output(GPIOE, PORTE_CS, 1);
  pin_set_alternate_function(GPIOA, PORTA_SCLK, 5);
  pin_set_alternate_function(GPIOA, PORTA_MISO, 5);
  pin_set_alternate_function(GPIOB, PORTB_MOSI, 5);
  pin_set_output_speed(GPIOA, PORTA_SCLK, 3);
  pin_set_output_speed(GPIOB, PORTB_MOSI, 3);
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
  accel_write_reg(0x20, 0x87); // set max beef (1600 Hz)
  delay_us(100); // wake up plz
#ifdef PRINT_REGISTER_TABLE
  uint8_t info1 = accel_read_reg(0x0d);
  printf("info1 = 0x%02x\r\n", (unsigned)info1);
#endif
  float test_accel[3] = {0};
  for (int i = 0; i < 10; i++)
  {
    delay_ms(10);
    imu_poll_accels(test_accel);
  }
}

static void accel_write_reg(const uint8_t reg, const uint8_t val)
{
  accel_txrx(reg, 1, NULL, &val);
}

#ifdef PRINT_REGISTER_TABLE
static uint8_t accel_read_reg(const uint8_t reg)
{
  uint8_t reg_val = 0;
  accel_txrx(reg, 1, &reg_val, NULL);
  return reg_val;
}
#endif

static void accel_txrx(const uint8_t start_reg, 
                       const uint8_t num_regs,
                       uint8_t *rx,
                       const uint8_t *tx)
{
  pin_set_output_low(GPIOE, PORTE_CS);
  volatile uint8_t __attribute__((unused)) rxd = SPI1->DR; // flush first!
  SPI1->DR = (tx == NULL ? 0x80 : 0x00) | start_reg; // tx = NULL during reads
  while (!(SPI1->SR & SPI_SR_RXNE)) { } // wait for it...
  rxd = SPI1->DR; // flush garbage (first inbound byte)
  for (int i = 0; i < num_regs; i++)
  {
    while (!(SPI1->SR & SPI_SR_TXE)) { }
    SPI1->DR = tx ? tx[i] : 0; // start SPI txrx
    while (!(SPI1->SR & SPI_SR_RXNE)) { } // wait for it...
    if (rx)
      rx[i] = SPI1->DR; // save the data byte that we received
    else
      SPI1->DR; // drain buffer anyway
  }
  while (SPI1->SR & SPI_SR_BSY) { }
  delay_ns(10); // burn a few cycles (TODO: is this needed? scope it sometime.)
  pin_set_output_high(GPIOE, PORTE_CS);
}

bool imu_poll_accels(float *xyz)
{
  uint8_t raw_read[6];
  accel_txrx(0x28, 6, raw_read, NULL);
  int16_t raw_accel[3];
  for (int i = 0; i < 3; i++)
  {
    raw_accel[i] = raw_read[i*2] | (raw_read[i*2+1] << 8);
    xyz[i] = raw_accel[i] / 16384.0f;
  }
#ifdef VERBOSE_ACCEL
  printf("\n\nimu_poll_accels\r\n");
  printf("raw read: %02x%02x  %02x%02x  %02x%02x\r\n",
         raw_read[0], raw_read[1],
         raw_read[2], raw_read[3],
         raw_read[4], raw_read[5]);
  printf("raw accel: [%d %d %d]\n",
         raw_accel[0], raw_accel[1], raw_accel[2]);
#endif
  return true;
}

