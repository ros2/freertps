#ifndef NET_CONFIG_H
#define NET_CONFIG_H

// hard-coded ip for now... 192.168.1.99.  TODO: something smarter
#define FRUDP_IP4_ADDR 0xc0a80163
#define ENET_MAC { 0xa4, 0xf3, 0xc1, 0x0, 0x2, 0x0 };

/*
#ifndef ETH_IP_ADDR
  #define ETH_IP_ADDR 0x0a42b159
#endif
uint32_t g_host_ip_addr = 0x0a42b164; // TODO: overwrite this intelligently
*/

#endif
