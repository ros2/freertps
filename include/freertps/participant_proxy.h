#ifndef FR_PARTICIPANT_PROXY_H
#define FR_PARTICIPANT_PROXY_H

#include <stdint.h>
#include "freertps/container.h"
#include "freertps/locator.h"
#include "freertps/protocol_version.h"
#include "freertps/vendor_id.h"

typedef uint32_t fr_builtin_endpoint_set_t;

typedef struct fr_participant_proxy
{
  struct fr_protocol_version protocol_version;
  struct fr_guid_prefix guid_prefix;
  fr_vendor_id_t vendor_id;
  bool expects_inline_qos;
  fr_builtin_endpoint_set_t available_builtin_endpoints;
  struct fr_locator metatraffic_unicast_locator;
  struct fr_locator metatraffic_multicast_locator;
  struct fr_locator default_unicast_locator;
  struct fr_locator default_multicast_locator;
  uint32_t manual_liveliness_count;
  struct fr_duration lease_duration;
} fr_participant_proxy_t;

#endif
