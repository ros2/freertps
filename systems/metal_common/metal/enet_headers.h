#ifndef ENET_HEADERS_H
#define ENET_HEADERS_H

#define ENET_MAC_LEN 6
typedef struct enet_eth_header
{
  uint8_t  dest_addr[ENET_MAC_LEN];
  uint8_t  source_addr[ENET_MAC_LEN];
  uint16_t ethertype;
} __attribute__((packed)) enet_eth_header_t;

#define ENET_ETHERTYPE_IP    0x0800
#define ENET_ETHERTYPE_ARP   0x0806

typedef struct enet_ip_header
{
  struct   enet_eth_header eth;
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
} __attribute__((packed)) enet_ip_header_t;

#define ENET_IP_HEADER_LEN     5
#define ENET_IP_VERSION        4
#define ENET_IP_DONT_FRAGMENT  0x4000

#define ENET_IP_PROTO_ICMP  0x01
#define ENET_IP_PROTO_UDP   0x11
typedef struct enet_udp_header
{
  struct   enet_ip_header ip;
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t len;
  uint16_t checksum;
} __attribute__((packed)) enet_udp_header_t;

typedef struct enet_arp_pkt
{
  struct   enet_eth_header eth;
  uint16_t hw_type;
  uint16_t proto_type;
  uint8_t  hw_addr_len;
  uint8_t  proto_addr_len;
  uint16_t operation;
  uint8_t  sender_hw_addr[6];
  uint32_t sender_proto_addr;
  uint8_t  target_hw_addr[6];
  uint32_t target_proto_addr;
} __attribute__((packed)) enet_arp_pkt_t;
#define ENET_ARP_HW_ETHERNET 1
#define ENET_ARP_PROTO_IPV4  0x0800
#define ENET_ARP_OP_REQUEST  1
#define ENET_ARP_OP_RESPONSE 2

#define ENET_ICMP_MAX_DATA 200
typedef struct enet_icmp_header
{
  struct   enet_ip_header ip;
  uint8_t  type;
  uint8_t  code;
  uint16_t checksum;
  uint16_t id;
  uint16_t sequence;
} __attribute__((packed)) enet_icmp_header_t;
#define ENET_ICMP_ECHO_REPLY   0x00
#define ENET_ICMP_ECHO_REQUEST 0x08

#endif
