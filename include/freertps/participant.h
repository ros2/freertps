#ifndef FR_PARTICIPANT_H
#define FR_PARTICIPANT_H

#include <stdbool.h>
#include "freertps/container.h"
#include "freertps/locator.h"
#include "freertps/protocol_version.h"
#include "freertps/udp.h"
#include "freertps/vendor_id.h"

typedef struct fr_participant
{
  struct fr_guid_prefix guid_prefix;
  struct fr_protocol_version protocol_version;
  fr_vendor_id_t vendor_id;
  struct fr_container *default_unicast_locators;
  struct fr_container *default_multicast_locators;
  struct fr_container *matched_participants;
} fr_participant_t;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix);
bool fr_participant_create();

#endif
