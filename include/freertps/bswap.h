#ifndef FREERTPS_BSWAP_H
#define FREERTPS_BSWAP_H

#include <stdint.h>

// todo: something clever with inlining someday, if this matters
uint32_t freertps_htonl(uint32_t u);
uint16_t freertps_htons(uint16_t u);
uint32_t freertps_ntohl(uint32_t u);
uint16_t freertps_ntohs(uint16_t u);

#endif
