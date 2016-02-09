#include <string.h>
#include "freertps/message.h"
#include "freertps/participant.h"

struct fr_message *fr_message_init(struct fr_message *msg)
{
  msg->header.magic_word = 0x53505452;
  msg->header.protocol_version.major = FR_PROTOCOL_VERSION_MAJOR;
  msg->header.protocol_version.minor = FR_PROTOCOL_VERSION_MINOR;
  msg->header.vendor_id = FREERTPS_VENDOR_ID;
  memcpy(msg->header.guid_prefix.prefix,
         g_fr_participant.guid_prefix.prefix,
         FR_GUID_PREFIX_LEN);
  //g_fr_udp_tx_buf_wpos = 0;
  return msg;
}

