#ifndef FR_PARTICIPANT_H
#define FR_PARTICIPANT_H

#include "freertps/udp.h"
#include "freertps/locator.h"
#include <stdbool.h>

typedef struct fr_participant
{
  fr_pver_t pver;
  fr_vid_t vid;
  fr_guid_prefix_t guid_prefix;
  bool expects_inline_qos;
  fr_locator_t default_unicast_locator;
  fr_locator_t default_multicast_locator;
  fr_locator_t metatraffic_unicast_locator;
  fr_locator_t metatraffic_multicast_locator;
  fr_duration_t lease_duration;
  fr_builtin_endpoint_set_t builtin_endpoints;
} fr_participant_t;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix);
bool fr_participant_create();

#endif
