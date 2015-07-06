#include "freertps/udp.h"
#include "freertps/spdp.h"
#include "freertps/discovery.h"
#include "freertps/subscription.h"
#include "freertps/publisher.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "net.h"

////////////////////////////////////////////////////////////////////////////
// global constants
const frudp_entity_id_t g_frudp_entity_id_unknown = { .u = 0 };
frudp_config_t g_frudp_config;
const frudp_sequence_number_t g_frudp_sequence_number_unknown = { .high = -1, .low = 0 };

////////////////////////////////////////////////////////////////////////////
// local functions
static bool frudp_rx_submsg(frudp_receiver_state_t *rcvr,
                            const frudp_submsg_t *submsg);
#define RX_MSG_ARGS frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg
static bool frudp_rx_acknack       (RX_MSG_ARGS);
static bool frudp_rx_heartbeat     (RX_MSG_ARGS);
static bool frudp_rx_gap           (RX_MSG_ARGS);
static bool frudp_rx_info_ts       (RX_MSG_ARGS);
static bool frudp_rx_info_src      (RX_MSG_ARGS);
static bool frudp_rx_info_reply_ip4(RX_MSG_ARGS);
static bool frudp_rx_dst           (RX_MSG_ARGS);
static bool frudp_rx_reply         (RX_MSG_ARGS);
static bool frudp_rx_nack_frag     (RX_MSG_ARGS);
static bool frudp_rx_heartbeat_frag(RX_MSG_ARGS);
static bool frudp_rx_data          (RX_MSG_ARGS);
static bool frudp_rx_data_frag     (RX_MSG_ARGS);

void frudp_tx_acknack(const frudp_guid_prefix_t *guid_prefix,
                      const frudp_entity_id_t *reader_entity_id,
                      const frudp_guid_t *writer_guid,
                      const frudp_sequence_number_set_t *set);

//////////////////////////////////////////////////////////////////////////


//#define RX_VERBOSE

bool frudp_rx(const uint32_t src_addr, const uint16_t src_port,
              const uint32_t dst_addr, const uint16_t dst_port,
              const uint8_t *rx_data  , const uint16_t rx_len)
{
#ifdef RX_VERBOSE
  FREERTPS_INFO("freertps rx %d bytes\n", rx_len);
#endif
  const frudp_msg_t *msg = (frudp_msg_t *)rx_data;
  if (msg->header.magic_word != 0x53505452) // todo: care about endianness
    return false; // it wasn't RTPS. no soup for you.
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx proto ver %d.%d\n",
                msg->header.pver.major,
                msg->header.pver.minor);
#endif
  if (msg->header.pver.major != 2)
    return false; // we aren't cool enough to be oldschool
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx vendor 0x%04x = %s\n",
                (unsigned)ntohs(msg->header.vid),
                frudp_vendor(ntohs(msg->header.vid)));
#endif
  // initialize the receiver state
  frudp_receiver_state_t rcvr;
  rcvr.src_pver = msg->header.pver;
  rcvr.src_vid = msg->header.vid;

  bool our_guid = true;
  for (int i = 0; i < 12; i++)
    if (msg->header.guid_prefix.prefix[i] !=
        g_frudp_config.guid_prefix.prefix[i])
      our_guid = false;
  if (our_guid)
    return true; // don't process our own messages

  {
#ifdef RX_VERBOSE
    const uint8_t *p = msg->header.guid_prefix.prefix;
    printf("RTPS sender guid prefix = %02x%02x%02x%02x:"
                                     "%02x%02x%02x%02x:"
                                     "%02x%02x%02x%02x\n",
         p[0], p[1], p[2], p[3],
         p[4], p[5], p[6], p[7],
         p[8], p[9], p[10], p[11]);
#endif
  }

  memcpy(rcvr.src_guid_prefix.prefix,
         msg->header.guid_prefix.prefix,
         FRUDP_GUID_PREFIX_LEN);
  rcvr.have_timestamp = false;
  // process all the submessages
  for (const uint8_t *submsg_start = msg->submsgs;
       submsg_start < rx_data + rx_len;)
  {
    const frudp_submsg_t *submsg = (frudp_submsg_t *)submsg_start;
    frudp_rx_submsg(&rcvr, submsg);
    // todo: ensure alignment? if this isn't dword-aligned, we're hosed
    submsg_start += sizeof(frudp_submsg_header_t) + submsg->header.len;
  }
  return true;
}

static bool frudp_rx_submsg(frudp_receiver_state_t *rcvr,
                            const frudp_submsg_t *submsg)
{
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx submsg ID %d len %d\n",
                submsg->header.id,
                submsg->header.len);
#endif
  // dispatch to message handlers
  switch (submsg->header.id)
  {
    case 0x01: return true; // pad submessage. ignore (?)
    case FRUDP_SUBMSG_ID_ACKNACK:   return frudp_rx_acknack(rcvr, submsg);
    case FRUDP_SUBMSG_ID_HEARTBEAT: return frudp_rx_heartbeat(rcvr, submsg);
    case 0x08: return frudp_rx_gap(rcvr, submsg);
    case FRUDP_SUBMSG_ID_INFO_TS:   return frudp_rx_info_ts(rcvr, submsg);
    case 0x0c: return frudp_rx_info_src(rcvr, submsg);
    case 0x0d: return frudp_rx_info_reply_ip4(rcvr, submsg);
    case FRUDP_SUBMSG_ID_INFO_DEST: return frudp_rx_dst(rcvr, submsg);
    case 0x0f: return frudp_rx_reply(rcvr, submsg);
    case 0x12: return frudp_rx_nack_frag(rcvr, submsg);
    case 0x13: return frudp_rx_heartbeat_frag(rcvr, submsg);
    case FRUDP_SUBMSG_ID_DATA:      return frudp_rx_data(rcvr, submsg);
    case 0x16: return frudp_rx_data_frag(rcvr, submsg);
    default: return false;
  }
  //FREERTPS_INFO("rx
  return true;
}

static bool frudp_rx_acknack(RX_MSG_ARGS)
{
  frudp_submsg_acknack_t *m = (frudp_submsg_acknack_t *)submsg->contents;
  printf("  ACKNACK   reader = 0x%08x  writer = 0x%08x   %d -> %d\n",
         (unsigned)htonl(m->reader_id.u),
         (unsigned)htonl(m->writer_id.u),
         (int)m->reader_sn_state.bitmap_base.low,
         (int)(m->reader_sn_state.bitmap_base.low + 
               m->reader_sn_state.num_bits));
  frudp_publisher_t *pub = frudp_publisher_from_writer_id(m->writer_id);
  if (!pub)
  {
    printf("couldn't find pub for writer id 0x%08x\n", 
           (unsigned)htonl(m->writer_id.u));
    return true; // not sure what's happening.
  }
  else
    frudp_publisher_rx_acknack(pub, m, &rcvr->src_guid_prefix);
  return true;
}

static bool frudp_rx_heartbeat(RX_MSG_ARGS)
{
  // todo: care about endianness
  const bool f = submsg->header.flags & 0x02;
  //const bool l = submsg->header.flags & 0x04; // liveliness flag?
  frudp_submsg_heartbeat_t *hb = (frudp_submsg_heartbeat_t *)submsg;
#ifdef VERBOSE_HEARTBEAT
  printf("  HEARTBEAT reader = 0x%08x  writer = 0x%08x  %d -> %d\n",
         htonl(hb->reader_id.u),
         htonl(hb->writer_id.u),
         hb->first_sn.low,
         hb->last_sn.low);
#endif

  frudp_guid_t writer_guid;
  frudp_stuff_guid(&writer_guid, &rcvr->src_guid_prefix, &hb->writer_id);

  // spin through subscriptions and see if anyone is listening
  bool found = false;
  for (unsigned i = 0; i < g_frudp_num_matched_readers; i++)
  {
    frudp_matched_reader_t *match = &g_frudp_matched_readers[i];
    if (!frudp_guid_identical(&writer_guid, &match->writer_guid))
      continue;
    if (hb->reader_id.u != match->reader_entity_id.u)
      continue;
    found = true;

    //g_frudp_subs[i].heartbeat_cb(rcvr, hb);
    if (!f)
    {
      // we have to send an ACKNACK now
      frudp_sequence_number_set_32bits_t set;
      // todo: handle 64-bit sequence numbers
      set.bitmap_base.high = 0;
      if (match->max_rx_sn.low >= hb->last_sn.low) // we're up-to-date
      {
        set.bitmap_base.low = hb->first_sn.low + 1;
        set.num_bits = 0;
        set.bitmap = 0xffffffff;
      }
      else
      {
        set.bitmap_base.low = match->max_rx_sn.low + 1;
        set.num_bits = hb->last_sn.low - match->max_rx_sn.low - 1;
        if (set.num_bits > 31)
          set.num_bits = 31;
        set.bitmap = 0xffffffff;
      }
#ifdef VERBOSE_HEARTBEAT
      printf("    TX ACKNACK %d:%d\n",
             set.bitmap_base.low,
             set.bitmap_base.low + set.num_bits);
#endif
      frudp_tx_acknack(&rcvr->src_guid_prefix,
                       &match->reader_entity_id,
                       &match->writer_guid,
                       (frudp_sequence_number_set_t *)&set);
    }
    else
    {
#ifdef VERBOSE_HEARTBEAT
      printf("    FINAL flag not set in heartbeat; not going to tx acknack\n");
#endif
    }
  }
  if (!found)
  {
    printf("      couldn't find match for inbound heartbeat:\n");
    printf("         writer = ");
    frudp_print_guid(&writer_guid);
    printf("  reader entity = %08x", (unsigned)hb->reader_id.u);
  }
  return true;
}

static bool frudp_rx_gap(RX_MSG_ARGS)
{
  frudp_submsg_gap_t *gap = (frudp_submsg_gap_t *)submsg;
  printf("  GAP reader = 0x%08x writer = 0x%08x  %d -> %d\n",
         (unsigned)htonl(gap->reader_id.u),
         (unsigned)htonl(gap->writer_id.u),
         (int)gap->gap_start.low,
         (int)(gap->gap_end.bitmap_base.low + 
               gap->gap_end.num_bits));
  return true;
}

static bool frudp_rx_info_ts(RX_MSG_ARGS)
{
  const bool invalidate = submsg->header.flags & 0x02;
  if (invalidate)
  {
    rcvr->have_timestamp = false;
    rcvr->timestamp.seconds = -1;
    rcvr->timestamp.fraction = 0xffffffff;
  }
  else
  {
    rcvr->have_timestamp = true;
    // todo: care about alignment
    rcvr->timestamp = *((fr_time_t *)(submsg->contents));
    /*
    FREERTPS_INFO("info_ts rx timestamp %.6f\n",
                  (double)(rcvr->timestamp.seconds) +
                  ((double)(rcvr->timestamp.fraction)) / ULONG_MAX);
    */
  }
  return true;
}

static bool frudp_rx_info_src(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_info_reply_ip4(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_dst(RX_MSG_ARGS)
{
#ifdef VERBOSE_INFO_DEST
  frudp_submsg_info_dest_t *d = (frudp_submsg_info_dest_t *)submsg->contents;
  uint8_t *p = d->guid_prefix.prefix;
  printf("  INFO_DEST guid = %02x%02x%02x%02x:"
                            "%02x%02x%02x%02x:"
                            "%02x%02x%02x%02x\n",
         p[0], p[1], p[2], p[3],
         p[4], p[5], p[6], p[7],
         p[8], p[9], p[10], p[11]);
#endif
  return true;
}

static bool frudp_rx_reply(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_nack_frag(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_heartbeat_frag(RX_MSG_ARGS)
{
  return true;
}

#define  VERBOSE_DATA

static bool frudp_rx_data(RX_MSG_ARGS)
{
  frudp_submsg_data_t *data_submsg = (frudp_submsg_data_t *)submsg;
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx data flags = %d\n", 0x0f7 & submsg->header.flags);
#endif
  // todo: care about endianness
  const bool q = submsg->header.flags & 0x02;
  //const bool d = submsg->header.flags & 0x04; // no idea what this is
  const bool k = submsg->header.flags & 0x08;
  if (k)
  {
    FREERTPS_ERROR("ahhhh i don't know how to handle keyed data yet\n");
    return false;
  }
  uint8_t *inline_qos_start = (uint8_t *)(&data_submsg->octets_to_inline_qos) +
                              sizeof(data_submsg->octets_to_inline_qos) +
                              data_submsg->octets_to_inline_qos;
  uint8_t *data_start = inline_qos_start;
  if (q)
  {
    // first parse out the QoS parameters
    frudp_parameter_list_item_t *item = (frudp_parameter_list_item_t *)inline_qos_start;
    while ((uint8_t *)item < submsg->contents + submsg->header.len)
    {
#ifdef RX_VERBOSE
      FREERTPS_INFO("data inline QoS param 0x%x len %d\n", (unsigned)item->pid, item->len);
#endif
      const frudp_parameterid_t pid = item->pid;
      //const uint8_t *pval = item->value;
      // todo: process parameter value
      item = (frudp_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
      if (pid == FRUDP_PID_SENTINEL)
        break; // adios
    }
    data_start = (uint8_t *)item; // after a PID_SENTINEL, this is correct
  }
  const uint16_t scheme = ntohs(*((uint16_t *)data_start));
  //printf("rx scheme = 0x%04x\n", scheme);
  uint8_t *data = data_start + 4;
#ifdef VERBOSE_DATA
  printf("  DATA reader id = 0x%08x  writer id = 0x%08x  seq %d\n",
         (unsigned)htonl(data_submsg->reader_id.u),
         (unsigned)htonl(data_submsg->writer_id.u),
         (int)data_submsg->writer_sn.low);
#endif
  frudp_guid_t writer_guid;
  frudp_stuff_guid(&writer_guid, &rcvr->src_guid_prefix, &data_submsg->writer_id);
  // spin through subscriptions and see if anyone is listening
  for (unsigned i = 0; i < g_frudp_num_matched_readers; i++)
  {
    frudp_matched_reader_t *match = &g_frudp_matched_readers[i];
    printf("sub %d: writer = %08x, reader = %08x\n",
           (int)i,
           (unsigned)htonl(match->writer_guid.entity_id.u),
           (unsigned)htonl(match->reader_entity_id.u));
    // have to special-case the SPDP entity ID's, since they come in
    // with any GUID prefix and with either an unknown reader entity ID
    // or the unknown-reader entity ID
    bool spdp_match = data_submsg->writer_id.u  == g_spdp_writer_id.u &&
                      (match->reader_entity_id.u == g_spdp_reader_id.u ||
                       match->reader_entity_id.u == g_frudp_entity_id_unknown.u);
    if (!spdp_match &&
        !frudp_guid_identical(&writer_guid, &match->writer_guid))
        continue; // move along. no match here.
    /*
    if (sub->writer_id.u == data_submsg->writer_id.u &&
        (sub->reader_id.u == data_submsg->reader_id.u ||
         data_submsg->reader_id.u == g_frudp_entity_id_unknown.u))
    */
    {
      // update the max-received sequence number counter
      if (data_submsg->writer_sn.low > match->max_rx_sn.low) // todo: 64-bit
        match->max_rx_sn = data_submsg->writer_sn;
      if (match->data_cb)
        match->data_cb(rcvr, submsg, scheme, data);
      if (match->msg_cb)
        match->msg_cb(data);
    }
  }
  //FREERTPS_ERROR("  ahh unknown data scheme: 0x%04x\n", (unsigned)scheme);
  return true;
}

static bool frudp_rx_data_frag(RX_MSG_ARGS)
{
  // todo
  return true;
}

bool frudp_generic_init()
{
  FREERTPS_INFO("frudp_generic_init()\n");
  frudp_add_mcast_rx(htonl(FRUDP_DEFAULT_MCAST_GROUP),
                     frudp_mcast_builtin_port());
  frudp_add_mcast_rx(htonl(FRUDP_DEFAULT_MCAST_GROUP),
                     frudp_mcast_user_port());
  frudp_add_ucast_rx(frudp_ucast_user_port());
  return true;
}

uint16_t frudp_mcast_builtin_port()
{
  return FRUDP_PORT_PB +
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D0;
}

uint16_t frudp_ucast_builtin_port()
{
  return FRUDP_PORT_PB +
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D1 +
         FRUDP_PORT_PG * g_frudp_config.participant_id;
}

uint16_t frudp_mcast_user_port()
{
  return FRUDP_PORT_PB +
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D2;
}

uint16_t frudp_ucast_user_port()
{
  return FRUDP_PORT_PB +
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D3 +
         FRUDP_PORT_PG * g_frudp_config.participant_id;
}

const char *frudp_ip4_ntoa(const uint32_t addr)
{
  static char ntoa_buf[20];
  snprintf(ntoa_buf, sizeof(ntoa_buf), "%d.%d.%d.%d",
           (int)(addr      ) & 0xff,
           (int)(addr >>  8) & 0xff,
           (int)(addr >> 16) & 0xff,
           (int)(addr >> 24) & 0xff);
  return ntoa_buf;
}

bool frudp_parse_string(char *buf, uint32_t buf_len, frudp_rtps_string_t *s)
{
  int wpos = 0;
  for (; wpos < s->len && wpos < buf_len-1; wpos++)
    buf[wpos] = s->data[wpos];
  buf[wpos] = 0;
  if (wpos < buf_len - 1)
    return true;
  else
    return false; // couldn't fit entire string in buffer
}

frudp_msg_t *frudp_init_msg(frudp_msg_t *buf)
{
  frudp_msg_t *msg = (frudp_msg_t *)buf;
  msg->header.magic_word = 0x53505452;
  msg->header.pver.major = 2;
  msg->header.pver.minor = 1;
  msg->header.vid = FREERTPS_VENDOR_ID;
  memcpy(msg->header.guid_prefix.prefix,
         g_frudp_config.guid_prefix.prefix,
         FRUDP_GUID_PREFIX_LEN);
  g_frudp_discovery_tx_buf_wpos = 0;
  return msg;
}

void frudp_tx_acknack(const frudp_guid_prefix_t *guid_prefix,
                      const frudp_entity_id_t *reader_id,
                      const frudp_guid_t      *writer_guid,
                      const frudp_sequence_number_set_t *set)
{
  static int s_acknack_count = 1;
  // find the participant we are trying to talk to
  frudp_participant_t *part = frudp_participant_find(guid_prefix);
  if (!part)
  {
    FREERTPS_ERROR("tried to acknack an unknown participant\n");
    return; // woah.
  }
  frudp_msg_t *msg = (frudp_msg_t *)g_frudp_discovery_tx_buf;
  frudp_init_msg(msg);
  //printf("    about to tx acknack\n");
  frudp_submsg_t *dst_submsg = (frudp_submsg_t *)&msg->submsgs[0];
  dst_submsg->header.id = FRUDP_SUBMSG_ID_INFO_DEST;
  dst_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  dst_submsg->header.len = 12;
  memcpy(dst_submsg->contents, guid_prefix, FRUDP_GUID_PREFIX_LEN);
  frudp_submsg_t *acknack_submsg = (frudp_submsg_t *)(&msg->submsgs[16]);
  acknack_submsg->header.id = FRUDP_SUBMSG_ID_ACKNACK;
  acknack_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  acknack_submsg->header.len = 24 + (set->num_bits + 31)/32 * 4;
  frudp_submsg_acknack_t *acknack =
                            (frudp_submsg_acknack_t *)acknack_submsg->contents;
  acknack->reader_id = *reader_id;
  acknack->writer_id = writer_guid->entity_id;
  int sn_set_len = (set->num_bits + 31) / 32 * 4 + 12;
  memcpy(&acknack->reader_sn_state, set, sn_set_len);
  uint32_t *p_count = (uint32_t *)&acknack->reader_sn_state + sn_set_len / 4;
  *p_count = s_acknack_count++;
  uint8_t *p_next_submsg = (uint8_t *)p_count + 4;
  int payload_len = p_next_submsg - (uint8_t *)msg;
  frudp_tx(part->metatraffic_unicast_locator.addr.udp4.addr,
           part->metatraffic_unicast_locator.port,
           (const uint8_t *)msg, payload_len);
}
