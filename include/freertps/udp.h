#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include "freertps/freertps.h"
#include <stdint.h>

typedef uint8_t freertps_udp_guidprefix_t[8];

typedef struct
{
  freertps_udp_guidprefix_t guidprefix;
} freertps_udp_guid_t;

void freertps_udp_init();
void freertps_udp_fini();

// move these to a private header someday
void freertps_hal_udp_init();
void freertps_hal_udp_fini();
void freertps_hal_udp_add_mcast_rx_port(uint16_t port);

#endif

