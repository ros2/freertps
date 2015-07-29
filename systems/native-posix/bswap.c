#include "freertps/bswap.h"
#include <arpa/inet.h>

uint32_t freertps_htonl(uint32_t u) { return htonl(u); }
uint16_t freertps_htons(uint16_t u) { return htons(u); }
uint32_t freertps_ntohl(uint32_t u) { return ntohl(u); }
uint16_t freertps_ntohs(uint16_t u) { return ntohs(u); }
