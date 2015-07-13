#ifndef NET_H
#define NET_H

#include <stdint.h>

uint16_t htons(const uint16_t x);
uint32_t htonl(const uint32_t x);

#define ntohs htons
#define ntohl htonl

#endif
