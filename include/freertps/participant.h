#ifndef FR_PARTICIPANT_H
#define FR_PARTICIPANT_H

#include <stdbool.h>
#include "freertps/container.h"
#include "freertps/locator.h"
#include "freertps/protocol_version.h"
#include "freertps/udp.h"

typedef struct fr_participant
{
  struct fr_guid_prefix guid_prefix;
  struct fr_protocol_version protocol_version;
  struct fr_vendor_id vendor_id;
  struct fr_container *default_unicast_locators;
  struct fr_container *default_multicast_locators;
} fr_participant_t;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix);
bool fr_participant_create();

#endif
