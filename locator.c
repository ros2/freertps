#include <stdio.h>
#include <string.h>
#include "freertps/container.h"
#include "freertps/locator.h"

void locator_print(struct fr_locator *loc)
{
  // assume they are udp4 for now...
  if (loc->kind == FR_LOCATOR_KIND_UDPV4)
  {
    printf("udp://%d.%d.%d.%d",
        (loc->addr.udp4.addr >>  0) & 0xff,
        (loc->addr.udp4.addr >>  8) & 0xff,
        (loc->addr.udp4.addr >> 16) & 0xff,
        (loc->addr.udp4.addr >> 24) & 0xff);
  }
  else
    printf("(unknown)\n");
  printf(":%d\n", loc->port);
}

fr_rc_t locator_container_append(uint32_t udpv4, uint16_t port,
    struct fr_container *c)
{
  struct fr_locator loc;
  loc.kind = FR_LOCATOR_KIND_UDPV4;
  loc.port = port;
  memset(loc.addr.udp4.zeros, 0, 12);
  loc.addr.udp4.addr = udpv4;
  return fr_container_append(c, &loc, sizeof(struct fr_locator), 
      FR_CFLAGS_NONE);
}
