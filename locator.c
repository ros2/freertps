#include <stdio.h>
#include <string.h>
#include "freertps/container.h"
#include "freertps/locator.h"

void fr_locator_print(struct fr_locator *loc)
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

fr_rc_t fr_locator_container_append(uint32_t udp4, uint16_t port,
    struct fr_container *c)
{
  struct fr_locator loc;
  fr_locator_set_udp4(&loc, udp4, port);
  return fr_container_append(c, &loc, sizeof(struct fr_locator), 
      FR_CFLAGS_NONE);
}

void fr_locator_set_udp4(struct fr_locator *loc,
    const uint32_t udp4, const uint16_t port)
{
  if (!loc)
    return;
  loc->kind = FR_LOCATOR_KIND_UDPV4;
  memset(loc->addr.udp4.zeros, 0, 12);
  loc->addr.udp4.addr = udp4;
  loc->port = port;
}

bool fr_locator_identical(struct fr_locator *a, struct fr_locator *b)
{
  if (a == b)
    return true;
  if (!a || !b)
    return false;
  if (a->kind != b->kind)
    return false;
  if (a->port != b->port)
    return false;
  if (a->kind == FR_LOCATOR_KIND_UDPV4)
    return a->addr.udp4.addr == b->addr.udp4.addr;
  else
  {
    printf("fr_locator_identical() not implemented for locator kind %d\n",
        (int)a->kind);
    return false;
  }
}
