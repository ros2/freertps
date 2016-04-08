#include "metal/enet.h"
#include "metal/enet_config.h"
#include "freertps/udp.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertps/bswap.h"

#define ALIGN4 __attribute__((aligned(4)));

#define ENET_RXPOOL_LEN     8192
#define ENET_RXPOOL_NPTR      64
#define ENET_RXPOOL_OFFSET     2
static volatile uint8_t  g_enet_rxpool[ENET_RXPOOL_LEN] ALIGN4;
static volatile uint16_t g_enet_rxpool_wpos = ENET_RXPOOL_OFFSET;
static volatile uint8_t *g_enet_rxpool_start[ENET_RXPOOL_NPTR] ALIGN4;
static volatile uint16_t g_enet_rxpool_len[ENET_RXPOOL_NPTR] ALIGN4;
static volatile uint16_t g_enet_rxpool_ptrs_wpos;
static volatile uint16_t g_enet_rxpool_ptrs_rpos;

static uint16_t g_enet_allowed_udp_ports[ENET_MAX_ALLOWED_UDP_PORTS];
static uint16_t g_enet_allowed_udp_ports_wpos;

const uint8_t g_enet_mac[6] = ENET_MAC;
static uint8_t  g_enet_udpbuf[1500] __attribute__((aligned(8)));

///////////////////////////////////////////////////////////////////////////
// local functions
///////////////////////////////////////////////////////////////////////////

static bool enet_dispatch_eth(const uint8_t *data, const uint16_t len);
static bool enet_dispatch_ip(const uint8_t *data, const uint16_t len);
static bool enet_dispatch_udp(const uint8_t *data, const uint16_t len);
static bool enet_dispatch_arp(const uint8_t *data, const uint16_t len);
static bool enet_dispatch_icmp(uint8_t *data, const uint16_t len);

void enet_init(void)
{
  printf("enet_init()\r\n");
  // set up the RAM pool for reception
  for (int i = 0; i < ENET_RXPOOL_NPTR; i++)
    g_enet_rxpool_start[i] = &g_enet_rxpool[ENET_RXPOOL_OFFSET];
  // chain into the MAC-provided initialization
  enet_mac_init();
}

enet_link_status_t enet_get_link_status(void)
{
  uint16_t status = enet_read_phy_reg(0x01);
  //printf("PHY status = 0x%02x\r\n", status);
  if (status & 0x04)
    return ENET_LINK_UP;
  return ENET_LINK_DOWN;
}


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
  enet_udp_header_t *h = (enet_udp_header_t *)&g_enet_udpbuf[0];
  for (int i = 0; i < 6; i++)
  {
    h->ip.eth.dest_addr[i] = dest_mac[i];
    h->ip.eth.source_addr[i] = g_enet_mac[i];
  }
  h->ip.eth.ethertype = freertps_htons(ENET_ETHERTYPE_IP);
  h->ip.header_len = ENET_IP_HEADER_LEN;
  h->ip.version = ENET_IP_VERSION; // ipv4
  h->ip.ecn = 0;
  h->ip.diff_serv = 0;
  h->ip.len = freertps_htons(20 + 8 + payload_len);
  static uint16_t id_increment = 0;
  h->ip.id = 0x8000 + id_increment++;
  h->ip.flag_frag = freertps_htons(ENET_IP_DONT_FRAGMENT);
  h->ip.ttl = 10; // not sure here... probably will be used just for LAN, but...
  h->ip.proto = ENET_IP_PROTO_UDP;
  h->ip.checksum = 0; // will be filled by the ethernet TX machinery
  h->ip.dest_addr = dest_ip; //enet_htonl(dest_ip);
  h->ip.source_addr = freertps_htonl(source_ip); //); // todo: something else
  h->dest_port = freertps_htons(dest_port);
  h->source_port = freertps_htons(source_port); //1234;
  h->len = freertps_htons(8 + payload_len);
  h->checksum = 0; // will be filled by the ethernet TX machinery
  memcpy(g_enet_udpbuf + sizeof(enet_udp_header_t), payload, payload_len);
  enet_mac_tx_raw(g_enet_udpbuf, sizeof(enet_udp_header_t) + payload_len);
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
  enet_send_raw_packet(raw_test_pkt, 68); //sizeof(reg_idx) + payload_len);
  */
}

uint_fast8_t enet_process_rx_ring(void)
{
  //printf("enet_process_rx_ring()\r\n");
  uint_fast8_t num_pkts_rx = 0;
  while (g_enet_rxpool_ptrs_wpos != g_enet_rxpool_ptrs_rpos)
  {
    const uint16_t rp = g_enet_rxpool_ptrs_rpos;
    const uint8_t *start = (const uint8_t *)g_enet_rxpool_start[rp];
    const uint16_t len = g_enet_rxpool_len[rp];
    //printf("eth rxpool wpos = %d rpos = %d start %d len %d\r\n",
    //       g_enet_rxpool_ptrs_wpos,
    //       rp, start - g_enet_rxpool, len);
    // see if it's addressed to us
    const enet_eth_header_t *e = (const enet_eth_header_t *)start;
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
      num_pkts_rx += enet_dispatch_eth(start, len) ? 1 : 0;
    }
    if (++g_enet_rxpool_ptrs_rpos >= ENET_RXPOOL_NPTR)
      g_enet_rxpool_ptrs_rpos = 0;
  }
  //printf("leaving enet_process_rx_ring()\r\n");
  return num_pkts_rx;
}

void enet_rx_raw(const uint8_t *pkt, const uint16_t pkt_len)
{
  // this function just places the incoming packet in a circular buffer
  // we want the ethernet payload 32-bit aligned, so we'll offset the
  // writes into the rxpool buffer by 2 bytes (ETH_RXPOOL_OFFSET)
  // see if this packet will run off the end of the buffer. if so, wrap.
  if (g_enet_rxpool_wpos + pkt_len + ENET_RXPOOL_OFFSET >= ENET_RXPOOL_LEN)
    g_enet_rxpool_wpos = ENET_RXPOOL_OFFSET;
  const uint16_t wp = g_enet_rxpool_ptrs_wpos;
  g_enet_rxpool_start[wp] = &g_enet_rxpool[g_enet_rxpool_wpos];
  g_enet_rxpool_len[wp] = pkt_len;
  memcpy((void *)g_enet_rxpool_start[wp], pkt, pkt_len);
  //printf("ethernet rx %d into rxpool at 0x%08x\r\n", 
  //       rxn, (unsigned)g_enet_rxpool_start[wp]);
  g_enet_rxpool_ptrs_wpos++;
  if (g_enet_rxpool_ptrs_wpos >= ENET_RXPOOL_NPTR)
    g_enet_rxpool_ptrs_wpos = 0;

  // make sure we end up with the rxpool write pointer on a 2-byte offset 
  // address (to keep the ethernet payloads 4-byte aligned) by incrementing
  // the pointer by a multiple of 4
  g_enet_rxpool_wpos += ((pkt_len + 3) & ~0x3);
}

static bool enet_dispatch_eth(const uint8_t *data, const uint16_t len)
{
  // dispatch according to protocol
  const enet_eth_header_t *e = (const enet_eth_header_t *)data;
  const uint16_t ethertype = freertps_htons(e->ethertype);
  //printf("eth dispatch ethertype 0x%04x\r\n", (unsigned)ethertype);
  switch (ethertype)
  {
    case ENET_ETHERTYPE_IP:
      return enet_dispatch_ip(data, len);
    case ENET_ETHERTYPE_ARP:
      return enet_dispatch_arp(data, len);
    default:
      return false;
  }
}

static void enet_add_ip_header_checksum(enet_ip_header_t *ip)
{
  ip->checksum = 0;
  uint32_t sum = 0;
  for (int word_idx = 0; word_idx < 10; word_idx++)
  {
    uint16_t word = *((uint16_t *)ip + sizeof(enet_eth_header_t)/2 + word_idx);
    word = freertps_htons(word);
    sum += word;
    //printf("word %d: 0x%02x\r\n", word_idx, word);
  }
  sum += (sum >> 16);
  sum &= 0xffff;
  ip->checksum = (uint16_t)freertps_htons(~sum);
  //printf("ip header checksum: 0x%04x\r\n", ip->ip_checksum);
}

static bool enet_dispatch_icmp(uint8_t *data, const uint16_t len)
{
  //leds_toggle(LEDS_YELLOW);
  //printf("enet_icmp_rx\r\n");
  enet_icmp_header_t *icmp = (enet_icmp_header_t *)data;
  if (icmp->type != ENET_ICMP_ECHO_REQUEST)
    return false;
  static const int ENET_ICMP_RESPONSE_MAX_LEN = 300; // i have no idea
  uint8_t icmp_response_buf[ENET_ICMP_RESPONSE_MAX_LEN];
  uint16_t incoming_ip_len = freertps_htons(icmp->ip.len);
  uint16_t icmp_data_len = incoming_ip_len - 20 - 8; // everything after icmp
  if (icmp_data_len > ENET_ICMP_RESPONSE_MAX_LEN - sizeof(enet_icmp_header_t))
    icmp_data_len = ENET_ICMP_RESPONSE_MAX_LEN - sizeof(enet_icmp_header_t);
  enet_icmp_header_t *icmp_response = (enet_icmp_header_t *)icmp_response_buf;
  for (int i = 0; i < 6; i++)
  {
    icmp_response->ip.eth.dest_addr[i]   = icmp->ip.eth.source_addr[i];
    icmp_response->ip.eth.source_addr[i] = g_enet_mac[i];
  }
  icmp_response->ip.eth.ethertype = freertps_htons(ENET_ETHERTYPE_IP);
  icmp_response->ip.header_len = ENET_IP_HEADER_LEN;
  icmp_response->ip.version = ENET_IP_VERSION;
  icmp_response->ip.ecn = 0;
  icmp_response->ip.diff_serv = 0;
  icmp_response->ip.len = freertps_htons(incoming_ip_len);
  icmp_response->ip.id = 0x8000;
  icmp_response->ip.flag_frag = freertps_htons(ENET_IP_DONT_FRAGMENT);
  icmp_response->ip.ttl = icmp->ip.ttl;
  icmp_response->ip.proto = ENET_IP_PROTO_ICMP;
  icmp_response->ip.checksum = 0;
  icmp_response->ip.source_addr = freertps_htonl(FRUDP_IP4_ADDR);
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
    uint16_t word = *((uint16_t *)icmp_response + sizeof(enet_ip_header_t)/2 +
                      word_idx);
    word = freertps_htons(word);
    csum += word;
  }
  csum += (csum >> 16);
  csum &= 0xffff;
  icmp_response->checksum = freertps_htons(~csum);
  enet_mac_tx_raw(icmp_response_buf,
                  sizeof(enet_icmp_header_t) + icmp_data_len);
  return true;
}

static bool enet_dispatch_arp(const uint8_t *data, const uint16_t len)
{
  enet_arp_pkt_t *arp_pkt = (enet_arp_pkt_t *)data;
  if (freertps_htons(arp_pkt->hw_type) != ENET_ARP_HW_ETHERNET ||
      freertps_htons(arp_pkt->proto_type) != ENET_ARP_PROTO_IPV4)
  {
    printf("unknown ARP hw type (0x%x) or protocol type (0x%0x)\r\n",
           (unsigned)freertps_htons(arp_pkt->hw_type), 
           (unsigned)freertps_htons(arp_pkt->proto_type));
    return false; // this function only handles ARP for IPv4 over ethernet
  }
  uint16_t op = freertps_htons(arp_pkt->operation);
  //printf("ARP op = 0x%04x\r\n", (unsigned)op);
  if (op == ENET_ARP_OP_REQUEST)
  {
    int req_ip = freertps_htonl(arp_pkt->target_proto_addr);
    if (req_ip != FRUDP_IP4_ADDR)
    {
      printf("ignoring ARP request for 0x%08x\r\n", req_ip);
      return false;
    }
    //printf("ARP request for 0x%08x\r\n", req_ip);
    //const uint8_t *request_enet_addr = arp_pkt->sender_hw_addr;
    //const uint32_t *request_ip = htonl(arp_pkt->sender_proto_addr);
    enet_arp_pkt_t response;
    for (int i = 0; i < 6; i++)
    {
      response.eth.dest_addr[i]   = arp_pkt->sender_hw_addr[i];
      response.eth.source_addr[i] = g_enet_mac[i];
      response.sender_hw_addr[i]  = g_enet_mac[i];
      response.target_hw_addr[i]  = arp_pkt->sender_hw_addr[i];
    }
    response.eth.ethertype = freertps_htons(ENET_ETHERTYPE_ARP);
    response.sender_proto_addr = freertps_htonl(FRUDP_IP4_ADDR);
    response.target_proto_addr = arp_pkt->sender_proto_addr;
    response.hw_type = freertps_htons(ENET_ARP_HW_ETHERNET);
    response.proto_type = freertps_htons(ENET_ARP_PROTO_IPV4);
    response.hw_addr_len = 6; // ethernet address length
    response.proto_addr_len = 4; // IPv4 address length
    response.operation = freertps_htons(ENET_ARP_OP_RESPONSE);
    enet_mac_tx_raw((uint8_t *)&response, sizeof(response));
    return true;
  }
  else if (op == ENET_ARP_OP_RESPONSE)
  {
    printf("arp response rx\r\n");
    // todo: smarter ARP table
    /*
    if (arp_pkt->sender_proto_addr == enet_htonl(g_host_ip_addr))
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

static bool enet_dispatch_ip(const uint8_t *data, const uint16_t len)
{
  const enet_ip_header_t *ip = (const enet_ip_header_t *)data;
  if (ip->version != 4) // we only handle ipv4 (for now...)
    return false;
  // if it's unicast, verify our IP address, otherwise ignore the packet
  if (ip->eth.dest_addr[0] == g_enet_mac[0]) // todo: smarter MAC filter.
    if (ip->dest_addr != freertps_htonl(FRUDP_IP4_ADDR))
      return false;
  if (ip->proto == ENET_IP_PROTO_UDP)
    return enet_dispatch_udp(data, len);
  else if (ip->proto == ENET_IP_PROTO_ICMP)
    return enet_dispatch_icmp((uint8_t *)data, len);
  return false; // if we get here, we aren't smart enough to handle this packet
}

static bool enet_dispatch_udp(const uint8_t *data, const uint16_t len)
{
  const enet_udp_header_t *udp = (const enet_udp_header_t *)data;
  const uint16_t port = freertps_htons(udp->dest_port);
  const uint16_t payload_len = freertps_htons(udp->len) - 8;
  const uint8_t *payload = data + sizeof(enet_udp_header_t);
  //printf("  udp len: %d\r\n", udp_payload_len);
  if (payload_len > len - sizeof(enet_udp_header_t))
    return false; // ignore fragmented UDP packets.
  //printf("dispatch udp @ %8u\r\n", (unsigned)SYSTIME);

  // todo: more efficient filtering
  bool port_match = false;
  for (int i = 0; !port_match && i < g_enet_allowed_udp_ports_wpos; i++)
    if (port == g_enet_allowed_udp_ports[i])
      port_match = true;
  // it would be nicer to have a callback mechanism here. someday.....
  if (port_match)
  {
    frudp_rx(udp->ip.source_addr, freertps_htonl(udp->source_port),
             udp->ip.dest_addr, freertps_htonl(udp->dest_port),
             payload, payload_len);
    return true;
  }

/*
  if (port == frudp_ucast_builtin_port() ||
      port == frudp_mcast_builtin_port() ||
      port == frudp_ucast_user_port()    ||
      port == frudp_mcast_user_port())
*/
  printf("unhandled udp: port = %d  payload_len = %d\r\n", port, payload_len);
  return false;
}

// todo: be smarter about multicast group choice
// old garbage:  #define MCAST_IP 0xe000008e

bool enet_allow_udp_port(const uint16_t port)
{
  // make sure we aren't already listening to this port
  printf("enet_allow_udp_port(%d)\r\n", port);
  for (int i = 0; i < ENET_MAX_ALLOWED_UDP_PORTS; i++)
    if (g_enet_allowed_udp_ports[i] == port)
      return true; // it's already allowed
  if (g_enet_allowed_udp_ports_wpos >= ENET_MAX_ALLOWED_UDP_PORTS)
    return false; // sorry, no room, have a nice day
  g_enet_allowed_udp_ports[g_enet_allowed_udp_ports_wpos++] = port;
  /*
  for (int i = 0; i < g_enet_allowed_udp_ports_wpos; i++)
    printf(" allowed port %d: %d\r\n", i, g_enet_allowed_udp_ports[i]);
  */
  return true;
}
