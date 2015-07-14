#include "freertps/bswap.h"

uint32_t freertps_htonl(uint32_t u) { return __REV(u); }
uint32_t freertps_ntohl(uint32_t u) { return __REV(u); }

uint16_t freertps_htons(uint16_t u) { return __REV16(u); }
uint16_t freertps_ntohs(uint16_t u) { return __REV16(u); }
