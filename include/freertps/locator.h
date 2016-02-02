#ifndef FR_LOCATOR_H
#define FR_LOCATOR_H

#include <stdint.h>

#define FR_LOCATOR_KIND_INVALID -1
#define FR_LOCATOR_KIND_RESERVED 0
#define FR_LOCATOR_KIND_UDPV4    1
#define FR_LOCATOR_KIND_UDPV6    2

typedef struct fr_locator
{
  int32_t kind;
  uint32_t port;
  union
  {
    uint8_t raw[16];
    struct
    {
      uint8_t zeros[12];
      uint32_t addr;
    } udp4;
  } addr; 
} __attribute__((packed)) fr_locator_t;

void locator_print(struct fr_locator *loc);

#endif
