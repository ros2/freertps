#ifndef FREERTPS_ENDPOINT_H
#define FREERTPS_ENDPOINT_H

#include "freertps/container.h"
#include "freertps/guid.h"

typedef struct fr_endpoint
{
  struct fr_guid guid;
  bool reliable;
  bool with_key;
  struct fr_container_t *unicast_locators;
  struct fr_container_t *multicast_locators;
} fr_endpoint_t;

#endif

