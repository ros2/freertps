#ifndef FREERTPS_RECEIVER_H
#define FREERTPS_RECEIVER_H

#include <stdbool.h>
#include "freertps/protocol_version.h"
#include "freertps/vendor_id.h"
#include "freertps/guid.h"
#include "freertps/time.h"

struct fr_receiver
{
  struct fr_protocol_version src_protocol_version;
  fr_vendor_id_t             src_vendor_id;
  struct fr_guid_prefix      src_guid_prefix;
  struct fr_guid_prefix      dst_guid_prefix;
  bool                       have_timestamp;
  struct fr_time             timestamp;
};

#endif
