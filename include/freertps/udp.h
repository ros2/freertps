#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include <stdint.h>

typedef uint8_t[8] freertps_udp_guidprefix_t;

typedef struct
{
  freertps_udp_guidprefix_t guidprefix;
} freertps_udp_guid_t;

#endif

