#include "enet.h"
#include <stdio.h>
#include <string.h>
#include "systime.h"
#include "delay.h"
#include "pin.h"
#include "net_config.h"
#include "freertps/udp.h"

// address is typically hard-wired to zero
#ifndef ENET_PHY_ADDR
#  define ENET_PHY_ADDR 0x00
#endif

#define ETH_NBUF     2048
#define ETH_DMA_NRXD   16
#define ETH_DMA_NTXD    4

// todo: parameterize this nicely for a makefile
const uint8_t g_enet_mac[6] = ENET_MAC;
//static uint8_t g_enet_master_mac[6] = {0};
//static bool g_enet_arp_valid = false;

typedef struct
{
  uint32_t des0;
  uint32_t des1;
  uint32_t des2;
  uint32_t des3;
} eth_dma_desc_t;

#define ALIGN4 __attribute__((aligned(4)));

static volatile eth_dma_desc_t g_eth_dma_rx_desc[ETH_DMA_NRXD] ALIGN4;
static volatile eth_dma_desc_t g_eth_dma_tx_desc[ETH_DMA_NTXD] ALIGN4;
static volatile uint8_t g_eth_dma_rx_buf[ETH_DMA_NRXD][ETH_NBUF] ALIGN4;
static volatile uint8_t g_eth_dma_tx_buf[ETH_DMA_NTXD][ETH_NBUF] ALIGN4;
static volatile eth_dma_desc_t *g_eth_dma_rx_next_desc = &g_eth_dma_rx_desc[0];
static volatile eth_dma_desc_t *g_eth_dma_tx_next_desc = &g_eth_dma_tx_desc[0];

#define ETH_RAM_RXPOOL_LEN  16384
#define ETH_RAM_RXPOOL_NPTR   128
#define ETH_RXPOOL_OFFSET 2
static volatile uint8_t  g_eth_rxpool[ETH_RAM_RXPOOL_LEN] ALIGN4;
static volatile uint16_t g_eth_rxpool_wpos = ETH_RXPOOL_OFFSET;
static volatile uint8_t *g_eth_rxpool_start[ETH_RAM_RXPOOL_NPTR] ALIGN4;
static volatile uint16_t g_eth_rxpool_len[ETH_RAM_RXPOOL_NPTR] ALIGN4;
static volatile uint16_t g_eth_rxpool_ptrs_wpos = 0;
static volatile uint16_t g_eth_rxpool_ptrs_rpos = 0;

///////////////////////////////////////////////////////////////////////////
// local functions
static void eth_send_raw_packet(uint8_t *pkt, uint16_t pkt_len);
static bool eth_dispatch_eth(const uint8_t *data, const uint16_t len);
static bool eth_dispatch_ip(const uint8_t *data, const uint16_t len);
static bool eth_dispatch_udp(const uint8_t *data, const uint16_t len);
static bool eth_dispatch_arp(const uint8_t *data, const uint16_t len);
static bool eth_dispatch_icmp(uint8_t *data, const uint16_t len);

///////////////////////////////////////////////////////////////////////////

uint16_t enet_read_phy_reg(const uint8_t reg_idx)
{
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // ensure MII is idle
  ETH->MACMIIAR = (ENET_PHY_ADDR << 11) |
                  ((reg_idx & 0x1f) << 6) |
                  ETH_MACMIIAR_CR_Div102  | // clock divider
                  ETH_MACMIIAR_MB;
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // spin waiting for MII to finish
  return ETH->MACMIIDR & 0xffff;
}

void enet_write_phy_reg(const uint8_t reg_idx, const uint16_t reg_val)
{
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // ensure MII is idle
  ETH->MACMIIDR = reg_val; // set the outgoing data word
  ETH->MACMIIAR = (ENET_PHY_ADDR << 11)   |
                  ((reg_idx & 0x1f) << 6) |
                  ETH_MACMIIAR_CR_Div102  | // MDC clock divider
                  ETH_MACMIIAR_MW         | // set the write bit
                  ETH_MACMIIAR_MB; // start it up
  while (ETH->MACMIIAR & ETH_MACMIIAR_MB) { } // spin waiting for MII to finish
  uint16_t readback_val = enet_read_phy_reg(reg_idx);
  if (readback_val != reg_val)
  {
    printf("woah there. tried to write 0x%04x to reg %02d but it read back %04x\r\n",
           reg_val, reg_idx, readback_val);
  }
}

void enet_init()
{
  printf("enet_init()\r\n");

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // enable the sysconfig block
  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
  for (volatile int i = 0; i < 1000; i++) { } // wait for sysconfig to come up
  // hold the MAC in reset while we set it to RMII mode
  for (volatile int i = 0; i < 1000; i++) { } // wait for sysconfig to come up
  SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL; // set the MAC in RMII mode
  for (volatile int i = 0; i < 100000; i++) { } // wait for sysconfig to come up
  RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACRXEN  |
                  RCC_AHB1ENR_ETHMACTXEN  |
                  RCC_AHB1ENR_ETHMACEN    ;  // turn on ur ethernet plz
  for (volatile int i = 0; i < 100000; i++) { } // wait
  RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST; // release MAC reset
  for (volatile int i = 0; i < 100000; i++) { } // wait
  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
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
  for (int i = 0; i < ETH_DMA_NTXD; i++)
  {
    g_eth_dma_tx_desc[i].des0 = 0x00100000 | // set address-chained bit
                                0x00c00000 ; // set insert-checksum bits
    g_eth_dma_tx_desc[i].des1 = 0;
    g_eth_dma_tx_desc[i].des2 = (uint32_t)&g_eth_dma_tx_buf[i][0]; // pointer to buf
    if (i < ETH_DMA_NTXD-1)
      g_eth_dma_tx_desc[i].des3 = (uint32_t)&g_eth_dma_tx_desc[i+1]; // chain to next
    else
      g_eth_dma_tx_desc[i].des3 = (uint32_t)&g_eth_dma_tx_desc[0]; // loop to first
  }
  ////////////////////////////////////////////////////////////////////////
  // set up ethernet RX descriptors
  for (int i = 0; i < ETH_DMA_NRXD; i++)
  {
    g_eth_dma_rx_desc[i].des0 = 0x80000000; // set "own" bit = DMA has control
    g_eth_dma_rx_desc[i].des1 = 0x00004000 | // set the RCH bit = chained addr2
                            ETH_NBUF; // buffer size in addr1
    g_eth_dma_rx_desc[i].des2 = (uint32_t)&g_eth_dma_rx_buf[i][0];
    if (i < ETH_DMA_NRXD-1)
      g_eth_dma_rx_desc[i].des3 = (uint32_t)&g_eth_dma_rx_desc[i+1];
    else
      g_eth_dma_rx_desc[i].des3 = (uint32_t)&g_eth_dma_rx_desc[0];
  }

  ///////////////////////////////////////////////////////////////////////
  // set up the RAM pool for reception
  for (int i = 0; i < ETH_RAM_RXPOOL_NPTR; i++)
  {
    g_eth_rxpool_start[i] = &g_eth_rxpool[0];
    g_eth_rxpool_len[i] = 0;
    g_eth_rxpool_ptrs_wpos = 0;
    g_eth_rxpool_ptrs_rpos = 0;
  }

  ///////////////////////////////////////////////////////////////////////
  // finally, turn on the DMA machinery
  ETH->DMATDLAR = (uint32_t)&g_eth_dma_tx_desc[0]; // point TX DMA to first desc
  ETH->DMARDLAR = (uint32_t)&g_eth_dma_rx_desc[0]; // point RX DMA to first desc
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

void eth_vector()
{
  volatile uint32_t dmasr = ETH->DMASR;
  ETH->DMASR = dmasr; // clear pending bits in the status register
  //printf("eth_vector()\r\n");
  if (dmasr & ETH_DMASR_RS)
  {
    // we received one or more frames. spin through and find them...
    while (!(g_eth_dma_rx_next_desc->des0 & 0x80000000))
    {
      // we want the ethernet payload 32-bit aligned, so we'll offset the
      // writes into the rxpool buffer by 2 bytes (ETH_RXPOOL_OFFSET)

      // todo: check all of the error status bits in des0...
      const uint16_t rxn = (g_eth_dma_rx_next_desc->des0 & 0x3fff0000) >> 16;
      // see if this packet will run off the end of the buffer. if so, wrap.
      if (g_eth_rxpool_wpos + rxn + ETH_RXPOOL_OFFSET >= ETH_RAM_RXPOOL_LEN)
        g_eth_rxpool_wpos = ETH_RXPOOL_OFFSET;
      const uint16_t wp = g_eth_rxpool_ptrs_wpos;
      g_eth_rxpool_start[wp] = &g_eth_rxpool[g_eth_rxpool_wpos];
      g_eth_rxpool_len[wp] = rxn;
      memcpy((void *)g_eth_rxpool_start[wp], 
             (const uint8_t *)g_eth_dma_rx_next_desc->des2,
             g_eth_rxpool_len[wp]);
      //printf("ethernet rx %d into rxpool at 0x%08x\r\n", 
      //       rxn, (unsigned)g_eth_rxpool_start[wp]);
      g_eth_rxpool_ptrs_wpos++;
      if (g_eth_rxpool_ptrs_wpos >= ETH_RAM_RXPOOL_NPTR)
        g_eth_rxpool_ptrs_wpos = 0;

      // make sure we end up with the rxpool write pointer on a 2-byte offset 
      // address (to keep the ethernet payloads 4-byte aligned) by incrementing
      // the pointer by a multiple of 4
      g_eth_rxpool_wpos += ((rxn+3) & ~0x3);
/*
      uint8_t *p = (uint8_t *)g_eth_rx_next_desc->des2;
      for (int i = 0; i < rxn; i++)
        printf("%02d: 0x%02x\r\n", i, p[i]);
*/
      g_eth_dma_rx_next_desc->des0 |= 0x80000000; // give it back to the DMA
      // advance the rx pointer for next time
      g_eth_dma_rx_next_desc = (eth_dma_desc_t *)g_eth_dma_rx_next_desc->des3;
    }
  }
  dmasr = ETH->DMASR;
  //printf("dmasr = 0x%08x\r\n", (int)dmasr);
}

enet_link_status_t enet_get_link_status()
{
  uint16_t status = enet_read_phy_reg(0x01);
  //printf("PHY status = 0x%02x\r\n", status);
  if (status & 0x04)
    return ENET_LINK_UP;
  return ENET_LINK_DOWN;
}

/*
static bool enet_master_mac_valid()
{
  for (int i = 0; i < 6; i++)
    if (g_enet_master_mac[i])
      return true;
  return false; // all zeros = invalid master MAC
}
*/

void eth_send_raw_packet(uint8_t *pkt, uint16_t pkt_len)
{
  //printf("eth tx %d\r\n", pkt_len);
  if (g_eth_dma_tx_next_desc->des0 & 0x80000000) // check the OWN bit
  {
    //printf("dma ring full. aborting transmission. dmasr = 0x%08lu\r\n",
    //        (uint32_t)ETH->DMASR);
    return; // if it's set, then we have run out of ringbuffer room. can't tx.
  }
  /*
  printf("sending using TX descriptor %08x status 0x%08x\r\n",
         (unsigned)g_eth_tx_next_desc,
         (unsigned)g_eth_tx_next_desc->control);
 */
  uint8_t *buf = (uint8_t *)g_eth_dma_tx_next_desc->des2;
  if (pkt_len > ETH_NBUF)
    pkt_len = ETH_NBUF; // let's not blow through our packet buffer
  memcpy(buf, pkt, pkt_len);
  g_eth_dma_tx_next_desc->des1 = pkt_len;
  g_eth_dma_tx_next_desc->des0 |= 0x30000000; // LS+FS = single-buffer packet
  g_eth_dma_tx_next_desc->des0 |= 0x80000000; // give ownership to ethernet DMA
  // see if DMA is stuck because it wasn't transmitting (which will almost
  // always be the case). if it's stuck, kick it into motion again
  if ((ETH->DMASR & ETH_DMASR_TPS) == ETH_DMASR_TPS_Suspended)
  {
    ETH->DMASR = ETH_DMASR_TBUS; // clear the buffer-unavailable flag
    ETH->DMATPDR = 0; // transmit poll demand = kick it moving again
  }
  g_eth_dma_tx_next_desc = (eth_dma_desc_t *)g_eth_dma_tx_next_desc->des3;
  //uint16_t r = enet_read_phy_reg(0x17);
  //printf(" rmii status = 0x%04x\r\n", (unsigned)r);
}

#define ETH_MAC_LEN 6
typedef struct
{
  uint8_t  dest_addr[ETH_MAC_LEN];
  uint8_t  source_addr[ETH_MAC_LEN];
  uint16_t ethertype : 16;
} __attribute__((packed)) eth_eth_header_t;

#define ETH_ETHERTYPE_IP    0x0800
#define ETH_ETHERTYPE_ARP   0x0806

typedef struct
{
  eth_eth_header_t eth;
  uint8_t  header_len   :  4;
  uint8_t  version      :  4;
  uint8_t  ecn          :  2;
  uint8_t  diff_serv    :  6;
  uint16_t len          : 16;
  uint16_t id           : 16;
  uint16_t flag_frag    : 16;
  uint8_t  ttl          :  8;
  uint8_t  proto        :  8;
  uint16_t checksum     : 16;
  uint32_t source_addr  : 32;
  uint32_t dest_addr    : 32;
} __attribute__((packed)) eth_ip_header_t;

#define ETH_IP_HEADER_LEN     5
#define ETH_IP_VERSION        4
#define ETH_IP_DONT_FRAGMENT  0x4000

#define ETH_IP_PROTO_ICMP  0x01
#define ETH_IP_PROTO_UDP   0x11

typedef struct
{
  eth_ip_header_t ip;
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t len;
  uint16_t checksum;
} __attribute__((packed)) eth_udp_header_t;

typedef struct
{
  eth_eth_header_t eth;
  uint16_t hw_type;
  uint16_t proto_type;
  uint8_t  hw_addr_len;
  uint8_t  proto_addr_len;
  uint16_t operation;
  uint8_t  sender_hw_addr[6];
  uint32_t sender_proto_addr;
  uint8_t  target_hw_addr[6];
  uint32_t target_proto_addr;
} __attribute__((packed)) eth_arp_pkt_t;
#define ETH_ETHERTYPE_ARP 0x0806
#define ETH_ARP_HW_ETHERNET 1
#define ETH_ARP_PROTO_IPV4 0x0800
#define ETH_ARP_OP_REQUEST 1
#define ETH_ARP_OP_RESPONSE 2

#define ICMP_MAX_DATA 200
typedef struct
{
  eth_ip_header_t ip;
  uint8_t  type;
  uint8_t  code;
  uint16_t checksum;
  uint16_t id;
  uint16_t sequence;
} __attribute__((packed)) enet_icmp_header_t;
static const uint8_t ENET_ICMP_ECHO_REPLY   = 0x00;
static const uint8_t ENET_ICMP_ECHO_REQUEST = 0x08;

static uint8_t  g_eth_udpbuf[1500] __attribute__((aligned(8))) = {0};

void enet_send_udp_mcast(const uint32_t mcast_ip, const uint16_t mcast_port,
                         const uint8_t *payload, const uint16_t payload_len)
{
  uint8_t dest_mac[6] = { 0x01, 0x00, 0x5e,
                          (uint8_t)((mcast_ip & 0xff0000) >> 16),
                          (uint8_t)((mcast_ip & 0x00ff00) >>  8),
                          (uint8_t) (mcast_ip & 0x0000ff) };
  enet_send_udp_ucast(dest_mac, mcast_ip, mcast_port,
                      FRUDP_IP4_ADDR, mcast_port,
                      payload, payload_len);
}

void enet_send_udp_ucast(const uint8_t *dest_mac,
                         const uint32_t dest_ip, const uint16_t dest_port,
                         const uint32_t source_ip, const uint16_t source_port,
                         const uint8_t *payload, const uint16_t payload_len)
{
  eth_udp_header_t *h = (eth_udp_header_t *)&g_eth_udpbuf[0];
  for (int i = 0; i < 6; i++)
  {
    h->ip.eth.dest_addr[i] = dest_mac[i];
    h->ip.eth.source_addr[i] = g_enet_mac[i];
  }
  h->ip.eth.ethertype = __REV16(ETH_ETHERTYPE_IP);
  h->ip.header_len = ETH_IP_HEADER_LEN;
  h->ip.version = ETH_IP_VERSION; // ipv4
  h->ip.ecn = 0;
  h->ip.diff_serv = 0;
  h->ip.len = __REV16(20 + 8 + payload_len);
  h->ip.id = 0;
  h->ip.flag_frag = __REV16(ETH_IP_DONT_FRAGMENT);
  h->ip.ttl = 1; // not sure here...
  h->ip.proto = ETH_IP_PROTO_UDP;
  h->ip.checksum = 0; // will be filled by the ethernet TX machinery
  h->ip.dest_addr = dest_ip; //eth_htonl(dest_ip);
  h->ip.source_addr = __REV(source_ip); //); // todo: something else
  h->dest_port = __REV16(dest_port);
  h->source_port = __REV16(source_port); //1234;
  h->len = __REV16(8 + payload_len);
  h->checksum = 0; // will be filled by the ethernet TX machinery
  memcpy(g_eth_udpbuf + sizeof(eth_udp_header_t), payload, payload_len);
  eth_send_raw_packet(g_eth_udpbuf, sizeof(eth_udp_header_t) + payload_len);
  /*
  uint8_t raw_test_pkt[128] =
  { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x7b, 0x90, 0x2b,
    0x34, 0x39, 0xb3, 0x2e, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x2b, 0x00, 0x00, 0x40, 0x00, 0x01, 0x11,
    0xd7, 0x1e, 0xc0, 0xa8, 0x01, 0x80, 0xe0, 0x00,
    0x00, 0x7b, 0xcc, 0xad, 0x04, 0xd4, 0x00, 0x17,
    0x00, 0x00, 0x01, 0x01, 0x00, 0x0b, 0x00, 0x00,
    0x08, 0xca, 0xfe, 0xbe, 0xef, 0x12, 0x34, 0x56,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7b, 0x94, 0x60, 0x0f };
  eth_send_raw_packet(raw_test_pkt, 68); //sizeof(reg_idx) + payload_len);
  */
}

uint_fast8_t enet_process_rx_ring()
{
  //printf("enet_process_rx_ring()\r\n");
  uint_fast8_t num_pkts_rx = 0;
  while (g_eth_rxpool_ptrs_wpos != g_eth_rxpool_ptrs_rpos)
  {
    const uint16_t rp = g_eth_rxpool_ptrs_rpos;
    const uint8_t *start = (const uint8_t *)g_eth_rxpool_start[rp];
    const uint16_t len = g_eth_rxpool_len[rp];
    //printf("eth rxpool wpos = %d rpos = %d start %d len %d\r\n",
    //       g_eth_rxpool_ptrs_wpos,
    //       rp, start - g_eth_rxpool, len);
    // see if it's addressed to us
    const eth_eth_header_t *e = (const eth_eth_header_t *)start;
    uint8_t unicast_match = 1, multicast_match = 1, broadcast_match = 1;
    /*
    printf("rx mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
           e->dest_addr[0], e->dest_addr[1], e->dest_addr[2],
           e->dest_addr[3], e->dest_addr[4], e->dest_addr[5]);
    */
    for (int i = 0; i < 6; i++)
    {
      if (e->dest_addr[i] != g_enet_mac[i])
        unicast_match = 0;
      if (e->dest_addr[i] != 0xff)
        broadcast_match = 0;
    }
    if (e->dest_addr[0] != 0x01 ||
        e->dest_addr[1] != 0x00 ||
        e->dest_addr[2] != 0x5e)
      multicast_match = 0;
    //printf("  ucast_match = %d, bcast_match = %d, mcast_match = %d\r\n",
    //       unicast_match, broadcast_match, multicast_match);
    if (unicast_match || multicast_match || broadcast_match)
    {
      //printf("eth dispatch @ %8u\r\n", (unsigned)SYSTIME);
      num_pkts_rx += eth_dispatch_eth(start, len) ? 1 : 0;
    }
    if (++g_eth_rxpool_ptrs_rpos >= ETH_RAM_RXPOOL_NPTR)
      g_eth_rxpool_ptrs_rpos = 0;
  }
  return num_pkts_rx;
}

static bool eth_dispatch_eth(const uint8_t *data, const uint16_t len)
{
  // dispatch according to protocol
  const eth_eth_header_t *e = (const eth_eth_header_t *)data;
  const uint16_t ethertype = __REV16(e->ethertype);
  //printf("eth dispatch ethertype 0x%04x\r\n", (unsigned)ethertype);
  switch (ethertype)
  {
    case ETH_ETHERTYPE_IP:
      return eth_dispatch_ip(data, len);
    case ETH_ETHERTYPE_ARP:
      return eth_dispatch_arp(data, len);
    default:
      return false;
  }
}

static void enet_add_ip_header_checksum(eth_ip_header_t *ip)
{
  ip->checksum = 0;
  uint32_t sum = 0;
  for (int word_idx = 0; word_idx < 10; word_idx++)
  {
    uint16_t word = *((uint16_t *)ip + sizeof(eth_eth_header_t)/2 + word_idx);
    word = __REV16(word);
    sum += word;
    //printf("word %d: 0x%02x\r\n", word_idx, word);
  }
  sum += (sum >> 16);
  sum &= 0xffff;
  ip->checksum = (uint16_t)__REV16(~sum);
  //printf("ip header checksum: 0x%04x\r\n", ip->ip_checksum);
}

static bool eth_dispatch_icmp(uint8_t *data, const uint16_t len)
{
  //leds_toggle(LEDS_YELLOW);
  //printf("enet_icmp_rx\r\n");
  enet_icmp_header_t *icmp = (enet_icmp_header_t *)data;
  if (icmp->type != ENET_ICMP_ECHO_REQUEST)
    return false;
  static const int ENET_ICMP_RESPONSE_MAX_LEN = 300; // i have no idea
  uint8_t icmp_response_buf[ENET_ICMP_RESPONSE_MAX_LEN];
  uint16_t incoming_ip_len = __REV16(icmp->ip.len);
  uint16_t icmp_data_len = incoming_ip_len - 20 - 8; // everything after icmp
  if (icmp_data_len > ENET_ICMP_RESPONSE_MAX_LEN - sizeof(enet_icmp_header_t))
    icmp_data_len = ENET_ICMP_RESPONSE_MAX_LEN - sizeof(enet_icmp_header_t);
  enet_icmp_header_t *icmp_response = (enet_icmp_header_t *)icmp_response_buf;
  for (int i = 0; i < 6; i++)
  {
    icmp_response->ip.eth.dest_addr[i]   = icmp->ip.eth.source_addr[i];
    icmp_response->ip.eth.source_addr[i] = g_enet_mac[i];
  }
  icmp_response->ip.eth.ethertype = __REV16(ETH_ETHERTYPE_IP);
  icmp_response->ip.header_len = ETH_IP_HEADER_LEN;
  icmp_response->ip.version = ETH_IP_VERSION;
  icmp_response->ip.ecn = 0;
  icmp_response->ip.diff_serv = 0;
  icmp_response->ip.len = __REV16(incoming_ip_len);
  icmp_response->ip.id = 0;
  icmp_response->ip.flag_frag = __REV16(ETH_IP_DONT_FRAGMENT);
  icmp_response->ip.ttl = icmp->ip.ttl;
  icmp_response->ip.proto = ETH_IP_PROTO_ICMP;
  icmp_response->ip.checksum = 0;
  icmp_response->ip.source_addr = __REV(FRUDP_IP4_ADDR);
  icmp_response->ip.dest_addr = icmp->ip.source_addr;
  icmp_response->type = ENET_ICMP_ECHO_REPLY;
  icmp_response->code = 0;
  icmp_response->checksum = 0;
  icmp_response->id = icmp->id;
  icmp_response->sequence = icmp->sequence;
  enet_add_ip_header_checksum(&icmp_response->ip);
  uint8_t *icmp_response_payload = icmp_response_buf + sizeof(enet_icmp_header_t);
  uint8_t *icmp_request_payload = data + sizeof(enet_icmp_header_t);
  for (int i = 0; i < icmp_data_len; i++)
    icmp_response_payload[i] = icmp_request_payload[i];
  uint32_t csum = 0;
  for (int word_idx = 0; word_idx < 4 + icmp_data_len / 2; word_idx++)
  {
    uint16_t word = *((uint16_t *)icmp_response + sizeof(eth_ip_header_t)/2 +
                      word_idx);
    word = __REV16(word);
    csum += word;
  }
  csum += (csum >> 16);
  csum &= 0xffff;
  icmp_response->checksum = __REV16(~csum);
  eth_send_raw_packet(icmp_response_buf,
                      sizeof(enet_icmp_header_t) + icmp_data_len);
  return true;
}

static bool eth_dispatch_arp(const uint8_t *data, const uint16_t len)
{
  eth_arp_pkt_t *arp_pkt = (eth_arp_pkt_t *)data;
  if (__REV16(arp_pkt->hw_type) != ETH_ARP_HW_ETHERNET ||
      __REV16(arp_pkt->proto_type) != ETH_ARP_PROTO_IPV4)
  {
    printf("unknown ARP hw type (0x%x) or protocol type (0x%0x)\r\n",
           (unsigned)__REV16(arp_pkt->hw_type), 
           (unsigned)__REV16(arp_pkt->proto_type));
    return false; // this function only handles ARP for IPv4 over ethernet
  }
  uint16_t op = __REV16(arp_pkt->operation);
  //printf("ARP op = 0x%04x\r\n", (unsigned)op);
  if (op == ETH_ARP_OP_REQUEST)
  {
    int req_ip = __REV(arp_pkt->target_proto_addr);
    if (req_ip != FRUDP_IP4_ADDR)
    {
      printf("ignoring ARP request for 0x%08x\r\n", req_ip);
      return false;
    }
    //printf("ARP request for 0x%08x\r\n", req_ip);
    //const uint8_t *request_eth_addr = arp_pkt->sender_hw_addr;
    //const uint32_t *request_ip = htonl(arp_pkt->sender_proto_addr);
    eth_arp_pkt_t response;
    for (int i = 0; i < 6; i++)
    {
      response.eth.dest_addr[i]   = arp_pkt->sender_hw_addr[i];
      response.eth.source_addr[i] = g_enet_mac[i];
      response.sender_hw_addr[i]  = g_enet_mac[i];
      response.target_hw_addr[i]  = arp_pkt->sender_hw_addr[i];
    }
    response.eth.ethertype = __REV16(ETH_ETHERTYPE_ARP);
    response.sender_proto_addr = __REV(FRUDP_IP4_ADDR);
    response.target_proto_addr = arp_pkt->sender_proto_addr;
    response.hw_type = __REV16(ETH_ARP_HW_ETHERNET);
    response.proto_type = __REV16(ETH_ARP_PROTO_IPV4);
    response.hw_addr_len = 6; // ethernet address length
    response.proto_addr_len = 4; // IPv4 address length
    response.operation = __REV16(ETH_ARP_OP_RESPONSE);
    eth_send_raw_packet((uint8_t *)&response, sizeof(response));
    return true;
  }
  else if (op == ETH_ARP_OP_RESPONSE)
  {
    printf("arp response rx\r\n");
    // todo: smarter ARP table
    /*
    if (arp_pkt->sender_proto_addr == eth_htonl(g_host_ip_addr))
    {
      for (int i = 0; i < 6; i++)
        g_enet_master_mac[i] = arp_pkt->sender_hw_addr[i];
      printf("master MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
             g_enet_master_mac[0], g_enet_master_mac[1],
             g_enet_master_mac[2], g_enet_master_mac[3],
             g_enet_master_mac[4], g_enet_master_mac[5]);
      g_enet_arp_valid = true;
    }
    */
    return true;
  }
  return false;
}

static bool eth_dispatch_ip(const uint8_t *data, const uint16_t len)
{
  const eth_ip_header_t *ip = (const eth_ip_header_t *)data;
  if (ip->version != 4) // we only handle ipv4 (for now...)
    return false;
  // if it's unicast, verify our IP address, otherwise ignore the packet
  if (ip->eth.dest_addr[0] == g_enet_mac[0]) // todo: smarter MAC filter.
    if (ip->dest_addr != __REV(FRUDP_IP4_ADDR))
      return false;
  if (ip->proto == ETH_IP_PROTO_UDP)
    return eth_dispatch_udp(data, len);
  else if (ip->proto == ETH_IP_PROTO_ICMP)
    return eth_dispatch_icmp((uint8_t *)data, len);
  return false; // if we get here, we aren't smart enough to handle this packet
}

static bool eth_dispatch_udp(const uint8_t *data, const uint16_t len)
{
  const eth_udp_header_t *udp = (const eth_udp_header_t *)data;
  const uint16_t port = __REV16(udp->dest_port);
  const uint16_t payload_len = __REV16(udp->len) - 8;
  const uint8_t *payload = data + sizeof(eth_udp_header_t);
  //printf("  udp len: %d\r\n", udp_payload_len);
  if (payload_len > len - sizeof(eth_udp_header_t))
    return false; // ignore fragmented UDP packets.
  //printf("dispatch udp @ %8u\r\n", (unsigned)SYSTIME);

  // todo: more efficient filtering
  if (port == frudp_ucast_builtin_port() ||
      port == frudp_mcast_builtin_port() ||
      port == frudp_ucast_user_port()    ||
      port == frudp_mcast_user_port())
  {
    frudp_rx(udp->ip.source_addr, __REV(udp->source_port),
             udp->ip.dest_addr, __REV(udp->dest_port),
             payload, payload_len);
    return true;
  }
  printf("unhandled udp: port = %d  payload_len = %d\r\n", port, payload_len);
  return false;
}

// todo: be smarter about multicast group choice
#define MCAST_IP 0xe000008e

#if 0
void enet_tx_state()
{
  //printf("%d enet_tx_state()\r\n", (int)SYSTIME);
  //volatile state_t tx_state = g_state; // make a local copy to ensure coherence
  //uint8_t fake_state[8] = {0};
  g_state.time_us = SYSTIME;
  g_state.channels_active = power_get_state();
  enet_send_udp_mcast(MCAST_IP, 11333,
                      (uint8_t *)&g_state, sizeof(g_state));
}
#endif

#if 0
void enet_systick()
{
  static int s_enet_systick_count = 0;
  if (++s_enet_systick_count % 2000 == 0 &&
      enet_get_link_status() == ENET_LINK_UP)
  {
    if (!enet_master_mac_valid())
    {
      //enet_request_master_mac();
    }
    //printf("enet 1hz systick\r\n");
  }
}
#endif
