#include "freertps/locator.h"
#include <stdio.h>

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
