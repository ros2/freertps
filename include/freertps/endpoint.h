#ifndef FREERTPS_ENDPOINT_H
#define FREERTPS_ENDPOINT_H

#include "freertps/container.h"
#include "freertps/guid.h"

typedef struct fr_endpoint
{
  union fr_entity_id entity_id; //struct fr_guid guid;
  bool reliable; // referred to as ReliabilityKind in the spec
  bool with_key; // referred to as TopicKind in the spec
  struct fr_container_t *unicast_locators;
  struct fr_container_t *multicast_locators;
} fr_endpoint_t;

void fr_endpoint_init(struct fr_endpoint *ep);

#endif

