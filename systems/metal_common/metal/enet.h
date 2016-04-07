#ifndef METAL_ENET_H
#define METAL_ENET_H

#include <stdint.h>
#include <stdbool.h>
#include "metal/enet_headers.h"

void enet_init(void);

extern const uint8_t g_enet_mac[6];
typedef enum { ENET_LINK_DOWN, ENET_LINK_UP } enet_link_status_t;
enet_link_status_t enet_get_link_status(void);

void enet_send_udp_ucast(const uint8_t *dest_mac,
                         const uint32_t dest_ip, const uint16_t dest_port,
                         const uint32_t source_ip, const uint16_t source_port,
                         const uint8_t *payload, const uint16_t payload_len);

void enet_send_udp_mcast(const uint32_t mcast_ip, const uint16_t mcast_port,
                         const uint8_t *payload, const uint16_t payload_len);

void enet_write_phy_reg(const uint8_t reg_idx, const uint16_t reg_val);
uint16_t enet_read_phy_reg(const uint8_t reg_idx);

void enet_rx_raw(const uint8_t *pkt, const uint16_t pkt_len); // enqueue it
uint_fast8_t enet_process_rx_ring(void); // process the rx queue

// for freertps, we only need 4 ports
#ifndef ENET_MAX_ALLOWED_UDP_PORTS
#define   ENET_MAX_ALLOWED_UDP_PORTS 4
#endif
bool enet_allow_udp_port(const uint16_t port);

/////////////////////////////////////////////////////////////////////////
// these functions must be provided by a chip-specific library
void enet_mac_init(void);
void enet_mac_tx_raw(const uint8_t *pkt, uint16_t pkt_len);

#endif
