#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "freertps/bswap.h"
#include "freertps/discovery.h"
#include "freertps/freertps.h"
#include "freertps/receiver.h"
#include "freertps/udp.h"
#include "freertps/spdp.h"
#include "freertps/participant.h"

////////////////////////////////////////////////////////////////////////////
// global constants
fr_config_t g_fr_config;
////////////////////////////////////////////////////////////////////////////
// local functions
//////////////////////////////////////////////////////////////////////////
//#define EXCESSIVELY_VERBOSE_MSG_RX
bool fr_udp_rx(const uint32_t src_addr, const uint16_t src_port,
               const uint32_t dst_addr, const uint16_t dst_port,
               const uint8_t *rx_data  , const uint16_t rx_len)
{
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
  printf("===============================================\n");
  printf("freertps rx %d bytes\n", rx_len);
  printf("===============================================\n");
#endif
  /*
  struct in_addr ina;
  ina.s_addr = dst_addr;
  printf("rx on %s:%d\n", inet_ntoa(ina), dst_port);
  */
  const struct fr_message *msg = (struct fr_message *)rx_data;
  if (msg->header.magic_word != 0x53505452) // todo: care about endianness
    return false; // it wasn't RTPS. no soup for you.
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
  FREERTPS_INFO("rx proto ver %d.%d\n",
                msg->header.pver.major,
                msg->header.pver.minor);
#endif
  if (msg->header.protocol_version.major != 2)
    return false; // we aren't cool enough to be oldschool
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
  FREERTPS_INFO("rx vendor 0x%04x = %s\n",
                (unsigned)ntohs(msg->header.vid),
                fr_vendor(ntohs(msg->header.vid)));
#endif
  // initialize the receiver state
  struct fr_receiver rcvr;
  rcvr.src_protocol_version = msg->header.protocol_version;
  rcvr.src_vendor_id = msg->header.vendor_id;

  bool our_guid = true;
  for (int i = 0; i < 12 && our_guid; i++)
    if (msg->header.guid_prefix.prefix[i] !=
        g_fr_participant.guid_prefix.prefix[i])
      our_guid = false;
  if (our_guid)
    return true; // don't process our own messages

  // hmm... p.35 says we should be initializing src_guid_prefix to UNKNOWN...
  memcpy(rcvr.src_guid_prefix.prefix,
         msg->header.guid_prefix.prefix,
         FR_GUID_PREFIX_LEN);
  memcpy(rcvr.dst_guid_prefix.prefix,
         g_fr_participant.guid_prefix.prefix,
         FR_GUID_PREFIX_LEN);
  rcvr.have_timestamp = false;
  // process all the submessages
  for (const uint8_t *submsg_start = msg->submessages;
       submsg_start < rx_data + rx_len;)
  {
    const struct fr_submessage *submsg = (struct fr_submessage *)submsg_start;
    fr_message_rx(&rcvr, submsg);
    // todo: ensure alignment? if this isn't dword-aligned, we're hosed
    submsg_start += sizeof(struct fr_submessage_header) + submsg->header.len;
  }
  return true;
}

const char *fr_ip4_ntoa(const uint32_t addr)
{
  static char ntoa_buf[20];
  snprintf(ntoa_buf, sizeof(ntoa_buf), "%d.%d.%d.%d",
           (int)(addr      ) & 0xff,
           (int)(addr >>  8) & 0xff,
           (int)(addr >> 16) & 0xff,
           (int)(addr >> 24) & 0xff);
  return ntoa_buf;
}

