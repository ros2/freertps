#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include "freertps/freertps.h"
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

typedef uint8_t freertps_udp_guidprefix_t[8];

typedef struct
{
  freertps_udp_guidprefix_t guidprefix;
} freertps_udp_guid_t;

bool freertps_udp_init();
void freertps_udp_fini();

typedef void (*freertps_udp_rx_callback_t)(const in_addr_t src_addr,
                                           const in_port_t src_port,
                                           const uint8_t *rx_data, 
                                           const uint16_t rx_len);

bool freertps_udp_add_mcast_rx(const in_addr_t group, 
                               const uint16_t port,
                               const freertps_udp_rx_callback_t rx_cb);

bool freertps_udp_listen(const uint32_t max_usec);

#endif

