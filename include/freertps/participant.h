#ifndef FRUDP_PARTICIPANT_H
#define FRUDP_PARTICIPANT_H

#include "freertps/udp.h"
#include <stdbool.h>

typedef struct
{
  frudp_pver_t pver;
  frudp_vid_t vid;
  frudp_guid_prefix_t guid_prefix;
  bool expects_inline_qos;
  frudp_locator_t default_unicast_locator;
  frudp_locator_t default_multicast_locator;
  frudp_locator_t metatraffic_unicast_locator;
  frudp_locator_t metatraffic_multicast_locator;
  frudp_duration_t lease_duration;
  frudp_builtin_endpoint_set_t builtin_endpoints;
} frudp_participant_t;

frudp_participant_t *frudp_participant_find(frudp_guid_prefix_t *guid_prefix);

#endif
