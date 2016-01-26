#ifndef FREERTPS_RECEIVER_H
#define FREERTPS_RECEIVER_H

#include <stdbool.h>
#include "freertps/protocol_version.h"
#include "freertps/vendor_id.h"
#include "freertps/guid.h"
#include "freertps/time.h"

typedef struct fr_receiver
{
  struct fr_protocol_version src_protocol_version;
  fr_vendor_id_t             src_vendor_id;
  fr_guid_prefix_t           src_guid_prefix;
  fr_guid_prefix_t           dst_guid_prefix;
  bool                       have_timestamp;
  fr_time_t                  timestamp;
} fr_receiver_t;

#endif
