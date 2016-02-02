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
static bool fr_rx_submsg(struct fr_receiver *rcvr,
                         const struct fr_submessage *submsg);
#define RX_MSG_ARGS struct fr_receiver *rcvr, const struct fr_submessage *submsg
static bool fr_rx_acknack       (RX_MSG_ARGS);
static bool fr_rx_heartbeat     (RX_MSG_ARGS);
static bool fr_rx_gap           (RX_MSG_ARGS);
static bool fr_rx_info_ts       (RX_MSG_ARGS);
static bool fr_rx_info_src      (RX_MSG_ARGS);
static bool fr_rx_info_reply_ip4(RX_MSG_ARGS);
static bool fr_rx_dst           (RX_MSG_ARGS);
static bool fr_rx_reply         (RX_MSG_ARGS);
static bool fr_rx_nack_frag     (RX_MSG_ARGS);
static bool fr_rx_heartbeat_frag(RX_MSG_ARGS);
static bool fr_rx_data          (RX_MSG_ARGS);
static bool fr_rx_data_frag     (RX_MSG_ARGS);

void fr_tx_acknack(const fr_guid_prefix_t *guid_prefix,
                   const fr_entity_id_t *reader_entity_id,
                   const fr_guid_t *writer_guid,
                   const fr_sequence_number_set_t *set);

//////////////////////////////////////////////////////////////////////////


//#define EXCESSIVELY_VERBOSE_MSG_RX

bool fr_rx(const uint32_t src_addr, const uint16_t src_port,
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
  fr_receiver_t rcvr;
  rcvr.src_protocol_version = msg->header.protocol_version;
  rcvr.src_vendor_id = msg->header.vendor_id;

  bool our_guid = true;
  for (int i = 0; i < 12 && our_guid; i++)
    if (msg->header.guid_prefix.prefix[i] !=
        g_fr_participant.guid_prefix.prefix[i])
      our_guid = false;
  if (our_guid)
    return true; // don't process our own messages

  memcpy(rcvr.src_guid_prefix.prefix,
         msg->header.guid_prefix.prefix,
         FR_GUID_PREFIX_LEN);
  rcvr.have_timestamp = false;
  // process all the submessages
  for (const uint8_t *submsg_start = msg->submessages;
       submsg_start < rx_data + rx_len;)
  {
    const struct fr_submessage *submsg = (struct fr_submessage *)submsg_start;
    fr_rx_submsg(&rcvr, submsg);
    // todo: ensure alignment? if this isn't dword-aligned, we're hosed
    submsg_start += sizeof(struct fr_submessage_header) + submsg->header.len;
  }
  return true;
}

static bool fr_rx_submsg(fr_receiver_t *rcvr,
                         const struct fr_submessage *submsg)
{
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
  FREERTPS_INFO("rx submsg ID %d len %d\n",
                submsg->header.id,
                submsg->header.len);
#endif
  // dispatch to message handlers
  switch (submsg->header.id)
  {
    case 0x01: return true; // pad submessage. ignore (?)
    case FR_SUBMSG_ID_ACKNACK:   return fr_rx_acknack(rcvr, submsg);
    case FR_SUBMSG_ID_HEARTBEAT: return fr_rx_heartbeat(rcvr, submsg);
    case 0x08: return fr_rx_gap(rcvr, submsg);
    case FR_SUBMSG_ID_INFO_TS:   return fr_rx_info_ts(rcvr, submsg);
    case 0x0c: return fr_rx_info_src(rcvr, submsg);
    case 0x0d: return fr_rx_info_reply_ip4(rcvr, submsg);
    case FR_SUBMSG_ID_INFO_DEST: return fr_rx_dst(rcvr, submsg);
    case 0x0f: return fr_rx_reply(rcvr, submsg);
    case 0x12: return fr_rx_nack_frag(rcvr, submsg);
    case 0x13: return fr_rx_heartbeat_frag(rcvr, submsg);
    case FR_SUBMSG_ID_DATA:      return fr_rx_data(rcvr, submsg);
    case 0x16: return fr_rx_data_frag(rcvr, submsg);
    default: return false;
  }
  //FREERTPS_INFO("rx
  return true;
}

static bool fr_rx_acknack(RX_MSG_ARGS)
{
#ifdef VERBOSE_ACKNACK
  fr_submsg_acknack_t *m = (fr_submsg_acknack_t *)submsg->contents;
  printf("  ACKNACK   0x%08x => ", (unsigned)freertps_htonl(m->writer_id.u));
  fr_guid_t reader_guid;
  fr_stuff_guid(&reader_guid, &rcvr->src_guid_prefix, &m->reader_id);
  fr_print_guid(&reader_guid);
  printf("   %d -> %d\n",
         (int)m->reader_sn_state.bitmap_base.low,
         (int)(m->reader_sn_state.bitmap_base.low +
               m->reader_sn_state.num_bits));
#endif
#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
  fr_pub_t *pub = fr_pub_from_writer_id(m->writer_id);
  if (!pub)
  {
    printf("couldn't find pub for writer id 0x%08x\n",
           (unsigned)freertps_htonl(m->writer_id.u));
    return true; // not sure what's happening.
  }
  else
    fr_pub_rx_acknack(pub, m, &rcvr->src_guid_prefix);
#endif
  return true;
}

static bool fr_rx_heartbeat(RX_MSG_ARGS)
{
  // todo: care about endianness
#ifdef VERBOSE_HEARTBEAT
  const bool f = submsg->header.flags & 0x02;
  //const bool l = submsg->header.flags & 0x04; // liveliness flag?
  fr_submsg_heartbeat_t *hb = (fr_submsg_heartbeat_t *)submsg;
  fr_guid_t writer_guid;
  fr_stuff_guid(&writer_guid, &rcvr->src_guid_prefix, &hb->writer_id);
  printf("  HEARTBEAT   ");
  fr_print_guid(&writer_guid);
  printf(" => 0x%08x  %d -> %d\n",
         (unsigned)freertps_htonl(hb->reader_id.u),
         (unsigned)hb->first_sn.low,
         (unsigned)hb->last_sn.low);
#endif
  //fr_print_readers();

#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
  //printf("%d matched readers\n", (int)g_fr_num_readers);
  fr_reader_t *match = NULL;
  // spin through subscriptions and see if we've already matched a reader
  for (unsigned i = 0; !match && i < g_fr_num_readers; i++)
  {
    fr_reader_t *r = &g_fr_readers[i];
    if (fr_guid_identical(&writer_guid, &r->writer_guid) &&
        (hb->reader_id.u == r->reader_entity_id.u ||
         hb->reader_id.u == 0))
      match = r;
  }
  // else, if we have a subscription for this, initialize a reader
  if (!match)
  {
    for (unsigned i = 0; !match && i < g_fr_num_subs; i++)
    {
      fr_sub_t *sub = &g_fr_subs[i];
      if (sub->reader_entity_id.u == hb->reader_id.u)
      {
        fr_reader_t r;
        memcpy(&r.writer_guid, &writer_guid, sizeof(fr_guid_t));
        r.reliable = sub->reliable;
        r.reader_entity_id = hb->reader_id;
        r.max_rx_sn.high = 0;
        r.max_rx_sn.low = 0;
        r.data_cb = sub->data_cb;
        r.msg_cb = sub->msg_cb;
        match = &r;
        printf("adding reader due to heartbeat RX\n");
        fr_add_reader(&r);
      }
    }
  }

  if (match)
  {
    //g_fr_subs[i].heartbeat_cb(rcvr, hb);
    if (match->reliable && !f)
    {
      //printf("acknack requested in heartbeat\n");
      // we have to send an ACKNACK now
      fr_seq_num_set_32bits_t set;
      // todo: handle 64-bit sequence numbers
      set.bitmap_base.high = 0;
      if (match->max_rx_sn.low >= hb->last_sn.low) // we're up-to-date
      {
        //printf("hb up to date\n");
        set.bitmap_base.low = hb->first_sn.low + 1;
        set.num_bits = 0;
        set.bitmap = 0xffffffff;
      }
      else
      {
        //printf("hb acknack'ing multiple samples\n");
        set.bitmap_base.low = match->max_rx_sn.low + 1;
        set.num_bits = hb->last_sn.low - match->max_rx_sn.low - 1;
        if (set.num_bits > 31)
          set.num_bits = 31;
        set.bitmap = 0xffffffff;
      }
      fr_tx_acknack(&rcvr->src_guid_prefix,
                    &match->reader_entity_id,
                    &match->writer_guid,
                    (fr_seq_num_set_t *)&set);
    }
    else
    {
#ifdef VERBOSE_HEARTBEAT
      printf("    FINAL flag not set in heartbeat; not going to tx acknack\n");
#endif
    }
  }
  else
  {
    printf("      couldn't find match for inbound heartbeat:\n");
    printf("         ");
    fr_print_guid(&writer_guid);
    printf(" => %08x\n", (unsigned)freertps_htonl(hb->reader_id.u));
  }
#endif
  return true;
}

static bool fr_rx_gap(RX_MSG_ARGS)
{
#ifdef VERBOSE_GAP
  fr_submsg_gap_t *gap = (fr_submsg_gap_t *)submsg;
  printf("  GAP 0x%08x => 0x%08x  %d -> %d\n",
         (unsigned)freertps_htonl(gap->writer_id.u),
         (unsigned)freertps_htonl(gap->reader_id.u),
         (int)gap->gap_start.low,
         (int)(gap->gap_end.bitmap_base.low +
               gap->gap_end.num_bits));
#endif
  return true;
}

static bool fr_rx_info_ts(RX_MSG_ARGS)
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
    //memcpy("
    //printf("about to read %08x\r\n", (unsigned)submsg->contents);
    const fr_time_t * const t_msg = (const fr_time_t * const)submsg->contents;
    rcvr->timestamp = *t_msg; //*((fr_time_t *)(submsg->contents));
    /*
    FREERTPS_INFO("info_ts rx timestamp %.6f\n",
                  (double)(rcvr->timestamp.seconds) +
                  ((double)(rcvr->timestamp.fraction)) / ULONG_MAX);
    */
  }
  return true;
}

static bool fr_rx_info_src(RX_MSG_ARGS)
{
  return true;
}

static bool fr_rx_info_reply_ip4(RX_MSG_ARGS)
{
  return true;
}

static bool fr_rx_dst(RX_MSG_ARGS)
{
#ifdef VERBOSE_INFO_DEST
  fr_submsg_info_dest_t *d = (fr_submsg_info_dest_t *)submsg->contents;
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

static bool fr_rx_reply(RX_MSG_ARGS)
{
  return true;
}

static bool fr_rx_nack_frag(RX_MSG_ARGS)
{
  return true;
}

static bool fr_rx_heartbeat_frag(RX_MSG_ARGS)
{
  return true;
}

static bool fr_rx_data(RX_MSG_ARGS)
{
  struct fr_data_submessage *data_submsg = (struct fr_data_submessage *)submsg;
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
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
    fr_parameter_list_item_t *item = (fr_parameter_list_item_t *)inline_qos_start;
    while ((uint8_t *)item < submsg->contents + submsg->header.len)
    {
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
      FREERTPS_INFO("data inline QoS param 0x%x len %d\n", (unsigned)item->pid, item->len);
#endif
      const fr_parameter_id_t pid = item->pid;
      //const uint8_t *pval = item->value;
      // todo: process parameter value
      item = (fr_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
      if (pid == FR_PID_SENTINEL)
        break; // adios
    }
    data_start = (uint8_t *)item; // after a PID_SENTINEL, this is correct
  }
  const uint16_t scheme = freertps_ntohs(*((uint16_t *)data_start));
  //printf("rx scheme = 0x%04x\n", scheme);
  uint8_t *data = data_start + 4;
  fr_guid_t writer_guid;
  fr_stuff_guid(&writer_guid, &rcvr->src_guid_prefix, &data_submsg->writer_id);
#ifdef VERBOSE_DATA
  printf("  DATA ");
  fr_print_guid(&writer_guid);
  printf(" => 0x%08x  : %d\r\n",
         (unsigned)freertps_htonl(data_submsg->reader_id.u),
         (int)data_submsg->writer_sn.low);
#endif
  // special-case SEDP, since some SEDP broadcasts (e.g., from opensplice
  // sometimes (?)) seem to come with reader_id set to 0
  //fr_entity_id_t reader_id = data_submsg->reader_id;
  //if (data_submsg->writer_id.u == 0xc2030000)
  //  reader_id.u = 0xc7030000;
  // spin through subscriptions and see if anyone is listening
#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
  int num_matches_found = 0;
  for (unsigned i = 0; i < g_fr_num_readers; i++)
  {
    fr_reader_t *match = &g_fr_readers[i];
    /*
    printf("    sub %d: writer = ", (int)i); //%08x, reader = %08x\n",
    fr_print_guid(&match->writer_guid);
    printf(" => %08x\n", (unsigned)htonl(match->reader_entity_id.u));
    */
           //(unsigned)htonl(match->writer_guid.entity_id.u),

    // have to special-case the SPDP entity ID's, since they come in
    // with any GUID prefix and with either an unknown reader entity ID
    // or the unknown-reader entity ID
    bool spdp_match = data_submsg->writer_id.u  == g_spdp_writer_id.u &&
                      (match->reader_entity_id.u == g_spdp_reader_id.u ||
                       match->reader_entity_id.u == g_fr_entity_id_unknown.u);
    if (!spdp_match &&
        !fr_guid_identical(&writer_guid, &match->writer_guid))
        continue; // move along. no match here.
    /*
    if (sub->writer_id.u == data_submsg->writer_id.u &&
        (sub->reader_id.u == data_submsg->reader_id.u ||
         data_submsg->reader_id.u == g_fr_entity_id_unknown.u))
    */
    num_matches_found++;
    // update the max-received sequence number counter
    if (data_submsg->writer_sn.low > match->max_rx_sn.low) // todo: 64-bit
      match->max_rx_sn = data_submsg->writer_sn;
    if (match->data_cb)
      match->data_cb(rcvr, submsg, scheme, data);
    if (match->msg_cb)
      match->msg_cb(data);
  }
  if (!num_matches_found)
  {
    /*
    printf("  DATA ");
    printf(" => 0x%08x  : %d\n",
      (unsigned)freertps_htonl(data_submsg->reader_id.u),
      (int)data_submsg->writer_sn.low);
    */
    printf("    couldn't find a matched reader for this DATA:\n");
    printf("      ");
    fr_print_guid(&writer_guid);
    printf("\n");
    printf("    available readers:\n");
    for (unsigned i = 0; i < g_fr_num_readers; i++)
    {
      fr_reader_t *match = &g_fr_readers[i];
      printf("      writer = ");
      fr_print_guid(&match->writer_guid);
      printf(" => %08x\n", 
        (unsigned)freertps_htonl(match->reader_entity_id.u));
    }
  }
  //FREERTPS_ERROR("  ahh unknown data scheme: 0x%04x\n", (unsigned)scheme);
#endif
  return true;
}

static bool fr_rx_data_frag(RX_MSG_ARGS)
{
  // todo
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

static uint32_t g_fr_udp_tx_buf_wpos;
struct fr_message *fr_init_msg(struct fr_message *buf)
{
  struct fr_message *msg = (struct fr_message *)buf;
  msg->header.magic_word = 0x53505452;
  msg->header.protocol_version.major = 2;
  msg->header.protocol_version.minor = 1;
  msg->header.vendor_id = FREERTPS_VENDOR_ID;
  memcpy(msg->header.guid_prefix.prefix,
         g_fr_participant.guid_prefix.prefix,
         FR_GUID_PREFIX_LEN);
  g_fr_udp_tx_buf_wpos = 0;
  return msg;
}

static uint8_t g_fr_udp_tx_buf[1536]; //FR_DISCOVERY_TX_BUFLEN];
void fr_tx_acknack(const fr_guid_prefix_t *guid_prefix,
                   const fr_entity_id_t         *reader_id,
                   const fr_guid_t        *writer_guid,
                   const fr_sequence_number_set_t *set)
{
  #ifdef VERBOSE_TX_ACKNACK
        printf("    TX ACKNACK %d:%d\n",
               (int)set->bitmap_base.low,
               (int)(set->bitmap_base.low + set->num_bits));
  #endif
  static int s_acknack_count = 1;
  // find the participant we are trying to talk to
  fr_participant_t *part = fr_participant_find(guid_prefix);
  if (!part)
  {
    FREERTPS_ERROR("tried to acknack an unknown participant\n");
    return; // woah.
  }
  struct fr_message *msg = (struct fr_message *)g_fr_udp_tx_buf;
  fr_init_msg(msg);
  //printf("    about to tx acknack\n");
  struct fr_submessage *dst_submsg = 
      (struct fr_submessage *)&msg->submessages[0];
  dst_submsg->header.id = FR_SUBMSG_ID_INFO_DEST;
  dst_submsg->header.flags = FR_FLAGS_LITTLE_ENDIAN |
                             FR_FLAGS_ACKNACK_FINAL;
  dst_submsg->header.len = 12;
  memcpy(dst_submsg->contents, guid_prefix, FR_GUID_PREFIX_LEN);
  struct fr_submessage *acknack_submsg =
      (struct fr_submessage *)(&msg->submessages[16]);
  acknack_submsg->header.id = FR_SUBMSG_ID_ACKNACK;
  acknack_submsg->header.flags = FR_FLAGS_LITTLE_ENDIAN;
  acknack_submsg->header.len = 24 + (set->num_bits + 31)/32 * 4;
  struct fr_acknack_submessage *acknack =
      (struct fr_acknack_submessage *)acknack_submsg->contents;
  acknack->reader_id = *reader_id;
  acknack->writer_id = writer_guid->entity_id;
  int sn_set_len = (set->num_bits + 31) / 32 * 4 + 12;
  memcpy(&acknack->reader_sn_state, set, sn_set_len);
  uint32_t *p_count = (uint32_t *)&acknack->reader_sn_state + sn_set_len / 4;
  *p_count = s_acknack_count++;
  uint8_t *p_next_submsg = (uint8_t *)p_count + 4;
  int payload_len = p_next_submsg - (uint8_t *)msg;
#ifdef HORRIBLY_BROKEN
  fr_tx(part->metatraffic_unicast_locator.addr.udp4.addr,
           part->metatraffic_unicast_locator.port,
           (const uint8_t *)msg, payload_len);
#endif
}
