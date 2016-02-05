#ifndef FREERTPS_READER_PROXY_H
#define FREERTPS_READER_PROXY_H

#include "freertps/container.h"
#include "freertps/guid.h"
#include "freertps/locator.h"

typedef struct fr_reader_proxy
{
  struct fr_guid remote_reader_guid;
  bool expects_inline_qos;
  struct fr_container *unicast_locators;
  struct fr_container *multicast_locators;
} fr_reader_proxy_t;

#endif

