#ifndef ENET_H
#define ENET_H

#include <stdint.h>

void enet_init();

typedef enum { ENET_LINK_DOWN, ENET_LINK_UP } enet_link_status_t;
enet_link_status_t enet_get_link_status();

void enet_send_udp_ucast(const uint8_t *dest_mac,
                         const uint32_t dest_ip, const uint16_t dest_port,
                         const uint32_t source_ip, const uint16_t source_port,
                         const uint8_t *payload, const uint16_t payload_len);

void enet_send_udp_mcast(const uint32_t mcast_ip, const uint16_t mcast_port,
                         const uint8_t *payload, const uint16_t payload_len);

void enet_write_phy_reg(const uint8_t reg_idx, const uint16_t reg_val);

uint_fast8_t enet_process_rx_ring();

void enet_tx_state();

#endif

