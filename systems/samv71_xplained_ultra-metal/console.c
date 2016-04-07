#include "samv71q21.h"
#include <stdint.h>
#include <stdbool.h>
#include "metal/console.h"
#include "freertps/periph/led.h"

// hardware connections wired to EDBG:
// PB04 = USART1 TXD on peripheral D
// PA21 = USART1 RXD on peripheral A

#define CONSOLE_BAUD 1000000
static bool g_console_init_complete = false;

#define TX_PIN PIO_PB4D_TXD1
#define CONSOLE_USART USART1

void console_init(void)
{
  PMC->PMC_PCER0 = (1 << ID_USART1) | (1 << ID_PIOB); // enable clock gates
  PIOB->PIO_OER = TX_PIN; // output enable
  PIOB->PIO_PDR = TX_PIN; // disable pin gpio, enable peripheral on pin
  PIOB->PIO_ABCDSR[0] |= TX_PIN; // peripheral D
  PIOB->PIO_ABCDSR[1] |= TX_PIN; // peripheral D
  MATRIX->MATRIX_WPMR = MATRIX_WPMR_WPKEY_PASSWD; // unlock the matrix, neo!
  MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4; // use PB4 as i/o, not jtag

  CONSOLE_USART->US_CR = US_CR_RSTTX | US_CR_RSTRX |
                         US_CR_TXDIS | US_CR_RXDIS |
                         US_CR_RSTSTA;
  CONSOLE_USART->US_MR = US_MR_USART_MODE_NORMAL |
                         US_MR_CHRL_8_BIT        |
                         US_MR_PAR_NO            |
                         US_MR_USCLKS_MCK        | // use MCLK = 144 MHz
                         US_MR_OVER;
  CONSOLE_USART->US_IDR = 0xffffffff; // no interrupts for now.
  //CONSOLE_USART->US_BRGR = 18; // 144 MHz / 1 megabit / 8 = 18
  CONSOLE_USART->US_BRGR = 156; // 144 MHz / 115200 / 8 = 156.25
  CONSOLE_USART->US_CR = US_CR_TXEN;
  g_console_init_complete = true;
}

void console_send_block(const uint8_t *block, const uint32_t len)
{
  // if it ever matters, make this interrupt-driven or (bonus) dma driven
  if (!g_console_init_complete)
    console_init();
  while ((CONSOLE_USART->US_CSR & US_CSR_TXRDY) == 0) { }
  for (uint32_t i = 0; i < len; i++)
  {
    CONSOLE_USART->US_THR = block[i];
    while ((CONSOLE_USART->US_CSR & US_CSR_TXRDY) == 0) { }
  }
  while (!(CONSOLE_USART->US_CSR & US_CSR_TXEMPTY)) { } // busy-wait until done
}
