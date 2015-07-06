#include "net.h"

uint16_t htons(const uint16_t x)
{
  return ((x & 0xff) << 8) | ((x >> 8) & 0xff);
}

uint32_t htonl(const uint32_t x)
{
  return ((x & 0x000000ff) << 24)  |
         ((x & 0x0000ff00) << 8)   |
         ((x & 0x00ff0000) >> 8)   |
         ((x & 0xff000000) >> 24);
}

