#ifndef FRUDP_PART_H
#define FRUDP_PART_H

#include "freertps/udp.h"
#include "freertps/locator.h"
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
} frudp_part_t;

frudp_part_t *frudp_part_find(const frudp_guid_prefix_t *guid_prefix);
bool frudp_part_create(void);

#endif
