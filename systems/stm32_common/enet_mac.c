#include "enet_mac.h"
#include "metal/enet.h"
#include <stdio.h>
#include <string.h>
#include "metal/systime.h"
#include "metal/delay.h"
#include "pin.h"
//#include "metal/enet_config.h"
//#include "freertps/udp.h"

// address is typically hard-wired to zero
#ifndef ENET_PHY_ADDR
#  define ENET_PHY_ADDR 0x00
#endif

#define ENET_NBUF     2048
#define ENET_DMA_NRXD   16
#define ENET_DMA_NTXD    8

typedef struct enet_dma_desc
{
  volatile uint32_t des0;
  volatile uint32_t des1;
  volatile uint32_t des2;
  volatile uint32_t des3;
} enet_dma_desc_t;

#define ALIGN4 __attribute__((aligned(4)));

static volatile enet_dma_desc_t g_enet_dma_rx_desc[ENET_DMA_NRXD] ALIGN4;
static volatile enet_dma_desc_t g_enet_dma_tx_desc[ENET_DMA_NTXD] ALIGN4;
static volatile uint8_t g_enet_dma_rx_buf[ENET_DMA_NRXD][ENET_NBUF] ALIGN4;
static volatile uint8_t g_enet_dma_tx_buf[ENET_DMA_NTXD][ENET_NBUF] ALIGN4;
static volatile enet_dma_desc_t *g_enet_dma_rx_next_desc = &g_enet_dma_rx_desc[0];
static volatile enet_dma_desc_t *g_enet_dma_tx_next_desc = &g_enet_dma_tx_desc[0];

uint16_t enet_read_phy_reg(const uint8_t reg_idx)
{
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // ensure MII is idle
  ETH->MACMIIAR = (ENET_PHY_ADDR << 11) |
                  ((reg_idx & 0x1f) << 6) |
                  ETH_MACMIIAR_CR_Div102  | // clock divider
                  ETH_MACMIIAR_MB;
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // spin waiting for MII 
  return ETH->MACMIIDR & 0xffff;
}

void enet_write_phy_reg(const uint8_t reg_idx, const uint16_t reg_val)
{
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // ensure MII is idle
  ETH->MACMIIDR = reg_val; // set the outgoing data word
  ETH->MACMIIAR = (ENET_PHY_ADDR << 11)    |
                   ((reg_idx & 0x1f) << 6)  |
                   ETH_MACMIIAR_CR_Div102  | // MDC clock divider
                   ETH_MACMIIAR_MW         | // set the write bit
                   ETH_MACMIIAR_MB;          // start it up
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // spin waiting for MII
  uint16_t readback_val = enet_read_phy_reg(reg_idx);
  if (readback_val != reg_val)
  {
    printf("woah there. tried to write 0x%04x to reg %02d but it read back %04x\r\n",
           reg_val, reg_idx, readback_val);
  }
}

void enet_mac_init(void)
{
  printf("stm32 enet_mac_init()\r\n");
  enet_mac_init_pins();

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // enable the sysconfig block
  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
  for (volatile int i = 0; i < 1000; i++) { } // wait for sysconfig to come up
  // hold the MAC in reset while we (optionally) set it to RMII mode
  for (volatile int i = 0; i < 1000; i++) { } // wait for sysconfig to come up
#ifndef ENET_USE_MII
  SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL; // set the MAC in RMII mode
#endif
  for (volatile int i = 0; i < 100000; i++) { } // wait for sysconfig to come up
  RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACRXEN  |
                  RCC_AHB1ENR_ETHMACTXEN  |
                  RCC_AHB1ENR_ETHMACEN    ;  // turn on ur ethernet plz
  for (volatile int i = 0; i < 100000; i++) { } // wait
  RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST; // release MAC reset
  for (volatile int i = 0; i < 100000; i++) { } // wait
  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST; // what am i doing
  for (volatile int i = 0; i < 100000; i++) { } // wait
  RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST; // release MAC reset
  for (volatile int i = 0; i < 100000; i++) { } // wait for sysconfig ... (?)

  ETH->DMABMR |= ETH_DMABMR_SR;
  for (volatile uint32_t i = 0; i < 100000; i++) { }
  while (ETH->DMABMR & ETH_DMABMR_SR) { } // wait for it to reset
  for (volatile uint32_t i = 0; i < 100000; i++) { }
  ETH->DMAOMR |= ETH_DMAOMR_FTF; // flush DMA
  while (ETH->DMAOMR & ETH_DMAOMR_FTF) { } // wait for it to flush

  // now, configure the ethernet peripheral
  ETH->MACCR |= 0x02000000 | // CSTF = strip FCS. why isn't there a symbol ?
                ETH_MACCR_FES  | // enable 100 mbit mode
                ETH_MACCR_DM   | // full duplex
                ETH_MACCR_IPCO | // ipv4 checksum auto-generation for RX
                ETH_MACCR_APCS;  // automatically remove pad+CRC from frames
  ETH->MACFFR |= ETH_MACFFR_RA; // for now, don't try to filter in hardware
  delay_ms(1);
  printf("waiting for PHY to wake up...\r\n");
  while (enet_read_phy_reg(0) == 0xffff) { }
  printf("PHY is ready.\r\n");
  //delay_ms(100);
  //for (volatile uint32_t i = 0; i < 1000000; i++) { } // let it initialize
  //printf("setting software strap registers...\r\n");
  /*
  enet_write_phy_reg(0x09, 0x7821); // enable auto MDIX,
                                    // set INT/PWDN to be interrupt output
                                    // enable auto-negotiation
  enet_write_phy_reg(0x09, 0xf821); // exit software-strap mode
  enet_write_phy_reg(0x04, 0x0101); // only advertise 100-FD mode
  */

  // cycle through and read a bunch of PHY registers to make sure it's alive
  /*
  for (int i = 0; i < 32; i++)
    printf("PHY reg %02d = 0x%04x\r\n", i, enet_read_phy_reg(i));
  */

  ////////////////////////////////////////////////////////////////////////
  // set up ethernet TX descriptors
  for (int i = 0; i < ENET_DMA_NTXD; i++)
  {
    g_enet_dma_tx_desc[i].des0 = 0x00100000 | // set address-chained bit
                                0x00c00000 ; // set insert-checksum bits
    g_enet_dma_tx_desc[i].des1 = 0;
    g_enet_dma_tx_desc[i].des2 = (uint32_t)&g_enet_dma_tx_buf[i][0]; // pointer to buf
    if (i < ENET_DMA_NTXD-1)
      g_enet_dma_tx_desc[i].des3 = (uint32_t)&g_enet_dma_tx_desc[i+1]; // chain to next
    else
      g_enet_dma_tx_desc[i].des3 = (uint32_t)&g_enet_dma_tx_desc[0]; // loop to first
  }
  ////////////////////////////////////////////////////////////////////////
  // set up ethernet RX descriptors
  for (int i = 0; i < ENET_DMA_NRXD; i++)
  {
    g_enet_dma_rx_desc[i].des0 = 0x80000000; // set "own" bit = DMA has control
    g_enet_dma_rx_desc[i].des1 = 0x00004000 | // set the RCH bit = chained addr2
                            ENET_NBUF; // buffer size in addr1
    g_enet_dma_rx_desc[i].des2 = (uint32_t)&g_enet_dma_rx_buf[i][0];
    if (i < ENET_DMA_NRXD-1)
      g_enet_dma_rx_desc[i].des3 = (uint32_t)&g_enet_dma_rx_desc[i+1];
    else
      g_enet_dma_rx_desc[i].des3 = (uint32_t)&g_enet_dma_rx_desc[0];
  }

  ///////////////////////////////////////////////////////////////////////
  // finally, turn on the DMA machinery
  ETH->DMATDLAR = (uint32_t)&g_enet_dma_tx_desc[0]; // point TX DMA to first desc
  ETH->DMARDLAR = (uint32_t)&g_enet_dma_rx_desc[0]; // point RX DMA to first desc
  ETH->DMAOMR = ETH_DMAOMR_TSF; // enable store-and-forward mode
  ETH->DMABMR |= ETH_DMABMR_AAB; // allow misaligned DMA start addrs
  /*
  ETH->DMABMR = ETH_DMABMR_AAB | ETH_DMABMR_USP |
                ETH_DMABMR_RDP_1Beat | ETH_DMABMR_RTPR_1_1 |
                ETH_DMABMR_PBL_1Beat | ETH_DMABMR_EDE;
  */
  ETH->DMAIER = ETH_DMAIER_NISE | ETH_DMAIER_RIE;
  ETH->MACCR |= ETH_MACCR_TE | // enable transmitter
                ETH_MACCR_RE;  // enable receiver
  NVIC_SetPriority(ETH_IRQn, 3);
  NVIC_EnableIRQ(ETH_IRQn);
  ETH->DMAOMR |= ETH_DMAOMR_ST | ETH_DMAOMR_SR; // enable ethernet DMA tx/rx
}

// this *must* be named "eth_vector" to override the weak ISR symbol
void eth_vector(void)
{
  volatile uint32_t dmasr = ETH->DMASR;
  ETH->DMASR = dmasr; // clear pending bits in the status register
  if (dmasr & ETH_DMASR_RS)
  {
    // we received one or more frames. spin through and find them...
    while (!(g_enet_dma_rx_next_desc->des0 & 0x80000000))
    {
      // todo: check all of the error status bits in des0...
      const uint8_t *rxp = (const uint8_t *)g_enet_dma_rx_next_desc->des2;
      const uint16_t rxn = (g_enet_dma_rx_next_desc->des0 & 0x3fff0000) >> 16;
      enet_rx_raw(rxp, rxn);
      g_enet_dma_rx_next_desc->des0 |= 0x80000000; // give it back to the DMA
      // advance the rx pointer for next time
      g_enet_dma_rx_next_desc = (enet_dma_desc_t *)g_enet_dma_rx_next_desc->des3;
    }
  }
  dmasr = ETH->DMASR;
  //printf("dmasr = 0x%08x\r\n", (int)dmasr);
}

void enet_mac_tx_raw(const uint8_t *pkt, uint16_t pkt_len)
{
  __disable_irq();
  if (g_enet_dma_tx_next_desc->des0 & 0x80000000) // check the OWN bit
  {
    printf("dma ring full. aborting transmission.\r\n");// dmasr = 0x%08lu\r\n",
    //        (uint32_t)ETH->DMASR);
    __enable_irq();
    return; // if it's set, then we have run out of ringbuffer room. can't tx.
  }
  uint8_t *buf = (uint8_t *)g_enet_dma_tx_next_desc->des2;
  if (pkt_len > ENET_NBUF)
    pkt_len = ENET_NBUF; // let's not blow through our packet buffer
  memcpy(buf, pkt, pkt_len);
  g_enet_dma_tx_next_desc->des1  = pkt_len;
  g_enet_dma_tx_next_desc->des0 |= 0x30000000 | // LS+FS = single-buffer packet
                                   0x80000000 ; // give ownership to eth DMA
  delay_ns(1); // for unknown reasons, we need to have a function call here
               // to waste enough time for the DMA descriptor to synchronize 
               // i thought the d-cache was turned off by default, but who knows
               // what's going on here.
  // see if DMA is stuck because it wasn't transmitting (which will almost
  // always be the case). if it's stuck, kick it into motion again
  const volatile uint32_t tps = ETH->DMASR & ETH_DMASR_TPS;
  if (tps == ETH_DMASR_TPS_Suspended)
    ETH->DMATPDR = 0; // transmit poll demand = kick it moving again
  g_enet_dma_tx_next_desc = (enet_dma_desc_t *)g_enet_dma_tx_next_desc->des3;
  __enable_irq();
}
