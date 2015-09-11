/*  Software License Agreement (Apache License)
 *
 *  Copyright 2015 Open Source Robotics Foundation
 *  Author: Morgan Quigley
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "metal/enet.h"
#include "samv71q21.h"
#include <stdio.h>
#include <string.h>
#include "metal/enet.h"
#include "metal/enet_config.h"
#include "pin.h"

// PHY is KSZ8061RNBVA in RMII mode, wired to Peripheral A of port D:
//   PD0 = REFCLK
//   PD1 = TXEN
//   PD2 = TXD0
//   PD3 = TXD1
//   PD4 = RXDV
//   PD5 = RXD0
//   PD6 = RXD1
//   PD7 = RXER
//   PD8 = MDC
//   PD9 = MDIO
//   PC10 = RESET
// PHY address = 1

// structure definitions taken from ASF 3.5.1,  /sam/drivers/emac/emac.h
typedef struct emac_rx_descriptor {
  union emac_rx_addr {
    uint32_t val;
    struct emac_rx_addr_bm {
      uint32_t b_ownership:1, // user clear, GMAC sets this to 1 after rx
      b_wrap:1,   // marks last descriptor in receive buffer 
      addr_dw:30; // address in number of DW
    } bm;
  } addr; // address, wrap & ownership 
  union emac_rx_status {
    uint32_t val;
    struct emac_rx_status_bm {
      uint32_t len:12,       // Length of frame including FCS 
      offset:2,              // rx buf offset, 13:12 of frame len (jumbo frame)
      b_sof:1,               // Start of frame 
      b_eof:1,               // End of frame 
      b_cfi:1,               // Concatenation Format Indicator 
      vlan_priority:3,       // VLAN priority (if VLAN detected) 
      b_priority_detected:1, // Priority tag detected 
      b_vlan_detected:1,     // VLAN tag detected 
      b_type_id_match:1,     // Type ID match
      b_addr4match:1,        // Address register 4 match
      b_addr3match:1,        // Address register 3 match
      b_addr2match:1,        // Address register 2 match
      b_addr1match:1,        // Address register 1 match
      reserved:1,
      b_ext_addr_match:1,    // External address match
      b_uni_hash_match:1,    // Unicast hash match
      b_multi_hash_match:1,  // Multicast hash match
      b_boardcast_detect:1;  // Global broadcast address detected
    } bm;
  } status;
} __attribute__ ((packed, aligned(8))) emac_rx_descriptor_t; 

typedef struct emac_tx_descriptor {
  uint32_t addr;
  union emac_tx_status {
    uint32_t val;
    struct emac_tx_status_bm {
      uint32_t len:11, // length of frame
      reserved:4,
      b_last_buffer:1, // is last buffer in frame?
      b_no_crc:1,      // no crc
      reserved1:10,
      b_exhausted:1,   // buffer exhausted mid frame
      b_underrun:1,    // tx underrun
      b_error:1,       // retry fail... error
      b_wrap:1,        // ring buffer wraparound bit
      b_used:1;        // user clear, GMAC sets this to 1 after tx
    } bm;
  } status;
} __attribute__ ((packed, aligned(8))) emac_tx_descriptor_t;

#define ENET_MAX_PKT_SIZE 1550

////////////////////////////////////////////////////////////////////////////
// globals
#define ENET_RX_BUFFERS   16
#define ENET_RX_UNITSIZE 128
static volatile emac_rx_descriptor_t __attribute__((aligned(8)))
                   g_enet_rx_desc[ENET_RX_BUFFERS];
static volatile uint8_t __attribute__((aligned(8))) 
                   g_enet_rx_buf[ENET_RX_BUFFERS * ENET_RX_UNITSIZE];
static uint8_t __attribute__((aligned(8)))
                   g_enet_rx_full_packet[ENET_MAX_PKT_SIZE];

// keep the TX path simple for now. single big buffer.
#define ENET_TX_BUFFERS 1
#define ENET_TX_UNITSIZE ENET_MAX_PKT_SIZE
volatile static emac_tx_descriptor_t __attribute__((aligned(8)))
                   g_enet_tx_desc[ENET_TX_BUFFERS];
volatile static uint8_t __attribute__((aligned(8)))
                   g_enet_tx_buf[ENET_TX_BUFFERS * ENET_TX_UNITSIZE];
volatile static uint8_t __attribute__((aligned(8)))
                   g_enet_udp_tx_buf[ENET_MAX_PKT_SIZE];

void enet_write_phy_reg(const uint8_t reg_idx, const uint16_t reg_val)
{
}

uint16_t enet_read_phy_reg(const uint8_t reg_idx)
{
  return 0;
}
void enet_mac_init()
{
  printf("samv71 enet mac init\r\n");
  PMC->PMC_PCER0 |= (1 << ID_PIOD);
  PMC->PMC_PCER1 |= (1 << (ID_GMAC - 32));

  for (int i = 0; i < 10; i++)
  {
    pin_set_mux(PIOD, i, 0); // set peripheral A for PD0 -> PD9
  }
  GMAC->GMAC_NCR = 0; // disable everything
  GMAC->GMAC_IDR = 0xffffffff; // disable all interrupts
  GMAC->GMAC_NCR |= GMAC_NCR_CLRSTAT; // reset statistics
  GMAC->GMAC_RSR = GMAC_RSR_RXOVR | GMAC_RSR_REC | GMAC_RSR_BNA; // clear flags
  GMAC->GMAC_TSR = GMAC_TSR_UBR | GMAC_TSR_COL  | GMAC_TSR_RLE |
                   GMAC_TSR_TFC | GMAC_TSR_TXCOMP | GMAC_TSR_TXCOMP; // clear
  GMAC->GMAC_ISR; // drain interrupts
  GMAC->GMAC_NCFGR = GMAC_NCFGR_RFCS      |  // drop FCS from rx packets
                     GMAC_NCFGR_PEN       |  // obey pause frames
                     GMAC_NCFGR_CAF       |  // promiscuous mode
                     GMAC_NCFGR_SPD       |  // 100 megabit
                     GMAC_NCFGR_FD        |  // full duplex
                     GMAC_NCFGR_CLK_MCK_64;  // mdio clock = mdc / 64
  // todo: PHY initalization using GMAC_MAN to set it to address 1
  // look at MPE bit in GMAC_NCR
  // also see what's going on with the RESET pin

  for (int i = 0; i < ENET_RX_BUFFERS; i++)
  {
    g_enet_rx_desc[i].addr.val = (uint32_t)&g_enet_rx_buf[i * ENET_RX_UNITSIZE] 
                                 & 0xfffffffc; // make sure it's 8-byte aligned
    g_enet_rx_desc[i].status.val = 0; 
  }
  g_enet_rx_desc[ENET_RX_BUFFERS-1].addr.bm.b_wrap = 1; // end of ring buffer
  GMAC->GMAC_RBQB = (uint32_t)g_enet_rx_desc & 0xfffffffc;

  for (int i = 0; i < ENET_TX_BUFFERS; i++)
  {
    g_enet_tx_desc[i].addr = (uint32_t)g_enet_tx_buf;
    g_enet_tx_desc[i].status.val = 0; // clear all flags
    g_enet_tx_desc[i].status.bm.b_used = 1; // no need to send this guy
  }
  g_enet_tx_desc[ENET_TX_BUFFERS-1].status.bm.b_wrap = 1; // end of ring 
  GMAC->GMAC_TBQB = (uint32_t)g_enet_tx_desc;

  GMAC->GMAC_NCR |= GMAC_NCR_RXEN   | // enable receiver
                    GMAC_NCR_WESTAT | // enable stats
                    GMAC_NCR_TXEN;    // enable transmitter
  GMAC->GMAC_IER = GMAC_IER_RXUBR | // receive used bit read (overrun?)
                   GMAC_IER_ROVR  | // receive overrun
                   GMAC_IER_RCOMP ; // receive complete
  NVIC_SetPriority(GMAC_IRQn, 2);
  NVIC_EnableIRQ(GMAC_IRQn);
}

void gmac_vector()
{
  printf("gmac_vector()\r\n");
#if 0
  // read the flags to reset the interrupt 
  volatile uint32_t enet_isr = GMAC->GMAC_ISR;
  volatile uint32_t enet_rsr = GMAC->GMAC_RSR;
  volatile uint32_t enet_tsr = GMAC->GMAC_TSR;
  if ((enet_isr & GMAC_ISR_RCOMP) || (enet_rsr & GMAC_RSR_REC))
  {
    volatile uint32_t rsr_clear_flag = GMAC_RSR_REC;
    if (enet_rsr & GMAC_RSR_OVR)
      rsr_clear_flag |= GMAC_RSR_OVR;
    if (enet_rsr & GMAC_RSR_BNA)
      rsr_clear_flag |= GMAC_RSR_BNA;
    GMAC->GMAC_RSR = rsr_clear_flag;
    // spin through buffers and mark them as unowned
    // collect used buffers into single ethernet RX buffer
    static int s_rx_buf_idx = 0;
    static int s_rx_pkt_write_idx = 0;
    while (g_enet_rx_desc[s_rx_buf_idx].addr.bm.b_ownership)
    {
      volatile emac_rx_descriptor_t *desc = &g_enet_rx_desc[s_rx_buf_idx];
      uint8_t *buf = (uint8_t *)&g_enet_rx_buf[s_rx_buf_idx*ENET_RX_UNITSIZE];
      desc->addr.bm.b_ownership = 0; // clear buffer
      if (desc->status.bm.b_sof)
        s_rx_pkt_write_idx = 0; // ensure we reset this
      const int buf_data_len = !desc->status.bm.len ? ENET_RX_UNITSIZE :
                               (desc->status.bm.len - s_rx_pkt_write_idx);
      /*
      printf("%d owned, size %d, sof %d, eof %d\r\n", 
             s_rx_buf_idx, buf_len, desc->status.bm.b_sof, 
             desc->status.bm.b_eof);
      */
      if (buf_data_len > 0 && 
          s_rx_pkt_write_idx + buf_data_len < ENET_MAX_PKT_SIZE)
      {
        memcpy(&g_enet_rx_full_packet[s_rx_pkt_write_idx], buf, buf_data_len);
        s_rx_pkt_write_idx += buf_data_len;
      }
      else
      {
        printf("AAAHH enet rx buffer trashed\r\n");
        s_rx_pkt_write_idx = 0;
      }
      if (desc->status.bm.b_eof)
      {
        enet_rx_raw(g_enet_rx_full_packet, s_rx_pkt_write_idx);
        s_rx_pkt_write_idx = 0; // to be really^n sure this gets reset... 
      }
      s_rx_buf_idx = ++s_rx_buf_idx % ENET_RX_BUFFERS; // advance in ring
    }
  }
#endif
}

void enet_mac_tx_raw(const uint8_t *pkt, uint16_t pkt_len)
{
#if 0
  g_enet_tx_desc[0].status.bm.b_used = 0; // we're monkeying with it
  // for now, we just crush whatever is in the tx buffer.
  if (pkt_len > ENET_TX_UNITSIZE)
    pkt_len = ENET_TX_UNITSIZE; // save memory from being brutally crushed
  /*
  printf("enet_tx_raw %d bytes:\r\n", pkt_len)
  for (int i = 0; i < pkt_len; i++)
    printf("%d: 0x%02x\r\n", i, pkt[i]);
  */
  memcpy((uint8_t *)g_enet_tx_buf, pkt, pkt_len);
  g_enet_tx_desc[0].status.bm.b_last_buffer = 1;
  g_enet_tx_desc[0].status.bm.len = pkt_len;
  GMAC->GMAC_NCR |= GMAC_NCR_TSTART; // kick off TX DMA
#endif
}

/*
// this could be useful someday (?)
int8_t enet_tx_avail()
{
  return (g_enet_tx_desc[0].status.bm.b_last_buffer != 0);
}
*/
