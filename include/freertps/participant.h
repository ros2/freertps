#ifndef FR_PARTICIPANT_H
#define FR_PARTICIPANT_H

#include "freertps/udp.h"
#include "freertps/locator.h"
#include "freertps/protocol_version.h"
#include "freertps/container.h"
#include <stdbool.h>

typedef struct fr_participant
{
  fr_guid_prefix_t guid_prefix;
  struct fr_protocol_version protocol_version;
  fr_vendor_id_t vendor_id;
  struct fr_container_t *default_unicast_locators;
  struct fr_container_t *default_multicast_locators;
  /*
  fr_locator_t default_unicast_locator;
  fr_locator_t default_multicast_locator;
  fr_locator_t metatraffic_unicast_locator;
  fr_locator_t metatraffic_multicast_locator;
  */
  fr_duration_t lease_duration;
  fr_builtin_endpoint_set_t builtin_endpoints;
} fr_participant_t;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix);
bool fr_participant_create();

#endif
