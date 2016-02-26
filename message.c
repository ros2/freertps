#include <stdio.h>
#include <string.h>
#include "freertps/freertps.h"
#include "freertps/iterator.h"
#include "freertps/message.h"
#include "freertps/participant.h"
#include "freertps/spdp.h"
#include "freertps/writer_proxy.h"

#define RX_MSG_ARGS struct fr_receiver *rcvr, const struct fr_submessage *submsg
static void fr_message_rx_acknack       (RX_MSG_ARGS);
static void fr_message_rx_heartbeat     (RX_MSG_ARGS);
static void fr_message_rx_gap           (RX_MSG_ARGS);
static void fr_message_rx_info_ts       (RX_MSG_ARGS);
static void fr_message_rx_info_src      (RX_MSG_ARGS);
static void fr_message_rx_info_reply_ip4(RX_MSG_ARGS);
static void fr_message_rx_dst           (RX_MSG_ARGS);
static void fr_message_rx_reply         (RX_MSG_ARGS);
static void fr_message_rx_nack_frag     (RX_MSG_ARGS);
static void fr_message_rx_heartbeat_frag(RX_MSG_ARGS);
static void fr_message_rx_data          (RX_MSG_ARGS);
static void fr_message_rx_data_frag     (RX_MSG_ARGS);
/*
// currently unused but will need it again someday for reliable readers
static void fr_message_tx_acknack(const struct fr_guid_prefix *guid_prefix,
    const fr_entity_id_t *reader_entity_id,
    const struct fr_guid *writer_guid,
    const struct fr_sequence_number_set *set);
*/

////////////////////////////////////////////////////////////////////////

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

void fr_message_rx(struct fr_receiver *r,
    const struct fr_submessage *m)
{
  FREERTPS_INFO("rx submsg ID 0x%02x len %d\n",
      (unsigned)m->header.id, m->header.len);
  // dispatch to message handlers; otherwise have to indent like crazy
  switch (m->header.id)
  {
    case 0x01:                        break; // pad submessage. ignore for now?
    case FR_SUBMSG_ID_ACKNACK:        fr_message_rx_acknack(r, m);       break;
    case FR_SUBMSG_ID_HEARTBEAT:      fr_message_rx_heartbeat(r, m);     break;
    case 0x08:                        fr_message_rx_gap(r, m);           break;
    case FR_SUBMSG_ID_INFO_TS:        fr_message_rx_info_ts(r, m);       break;
    case 0x0c:                        fr_message_rx_info_src(r, m);      break;
    case 0x0d:                        fr_message_rx_info_reply_ip4(r,m); break;
    case FR_SUBMSG_ID_INFO_DEST:      fr_message_rx_dst(r, m);           break;
    case 0x0f:                        fr_message_rx_reply(r, m);         break;
    case 0x12:                        fr_message_rx_nack_frag(r, m);     break;
    case FR_SUBMSG_ID_HEARTBEAT_FRAG: fr_message_rx_heartbeat_frag(r,m); break;
    case FR_SUBMSG_ID_DATA:           fr_message_rx_data(r, m);          break;
    case FR_SUBMSG_ID_DATA_FRAG:      fr_message_rx_data_frag(r, m);     break;
    default:                         
      FREERTPS_INFO("rx unknown submsg ID 0x%02x len %d\n",
          (unsigned)m->header.id, m->header.len);
      break;
  }
}

#define VERBOSE_ACKNACK
static void fr_message_rx_acknack(RX_MSG_ARGS)
{
#ifdef VERBOSE_ACKNACK
  struct fr_submessage_acknack *m =
      (struct fr_submessage_acknack *)submsg->contents;
  printf("  ACKNACK   0x%08x => ", (unsigned)freertps_htonl(m->writer_id.u));
  struct fr_guid reader_guid;
  fr_guid_stuff(&reader_guid, &rcvr->src_guid_prefix, &m->reader_id);
  fr_guid_print(&reader_guid);
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
  }
  else
    fr_pub_rx_acknack(pub, m, &rcvr->src_guid_prefix);
#endif
}

#define VERBOSE_HEARTBEAT
static void fr_message_rx_heartbeat(RX_MSG_ARGS)
{
  // todo: care about endianness
#ifdef VERBOSE_HEARTBEAT
  //const bool f = submsg->header.flags & 0x02;
  //const bool l = submsg->header.flags & 0x04; // liveliness flag?
  struct fr_submessage_heartbeat *hb = 
      (struct fr_submessage_heartbeat *)submsg;
  struct fr_guid writer_guid;
  fr_guid_stuff(&writer_guid, &rcvr->src_guid_prefix, &hb->writer_id);
  printf("  HEARTBEAT   ");
  fr_guid_print(&writer_guid);
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
}

static void fr_message_rx_gap(RX_MSG_ARGS)
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
}

static void fr_message_rx_info_ts(RX_MSG_ARGS)
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
    const struct fr_time * const t_msg =
        (const struct fr_time * const)submsg->contents;
    rcvr->timestamp = *t_msg; //*((fr_time_t *)(submsg->contents));
    /*
    FREERTPS_INFO("info_ts rx timestamp %.6f\n",
                  (double)(rcvr->timestamp.seconds) +
                  ((double)(rcvr->timestamp.fraction)) / ULONG_MAX);
    */
  }
}

static void fr_message_rx_info_src(RX_MSG_ARGS)
{
  printf("  INFO_SRC\n");
}

static void fr_message_rx_info_reply_ip4(RX_MSG_ARGS)
{
}

#define VERBOSE_INFO_DEST
static void fr_message_rx_dst(RX_MSG_ARGS)
{
#ifdef VERBOSE_INFO_DEST
  struct fr_submessage_info_dest *d =
      (struct fr_submessage_info_dest *)submsg->contents;
  uint8_t *p = d->guid_prefix.prefix;
  printf("  INFO_DEST guid = %02x%02x%02x%02x:"
                            "%02x%02x%02x%02x:"
                            "%02x%02x%02x%02x\n",
         p[0], p[1], p[2], p[3],
         p[4], p[5], p[6], p[7],
         p[8], p[9], p[10], p[11]);
#endif
}

static void fr_message_rx_reply(RX_MSG_ARGS)
{
}

static void fr_message_rx_nack_frag(RX_MSG_ARGS)
{
}

static void fr_message_rx_heartbeat_frag(RX_MSG_ARGS)
{
}

//#define EXCESSIVELY_VERBOSE_MSG_RX
#define VERBOSE_DATA
static void fr_message_rx_data(RX_MSG_ARGS)
{
  struct fr_submessage_data *data_submsg = (struct fr_submessage_data *)submsg;
#ifdef EXCESSIVELY_VERBOSE_MSG_RX
  FREERTPS_INFO("rx data flags = %d\n", 0x0f7 & submsg->header.flags);
#endif
  // todo: care about endianness
  const bool q = submsg->header.flags & 0x02;
  //const bool d = submsg->header.flags & 0x04; // no idea what this is
  const bool k = submsg->header.flags & 0x08;
  const bool little_endian = submsg->header.flags & 0x01;
  if (k)
  {
    FREERTPS_ERROR("ahhhh i don't know how to handle keyed data yet\n");
    return;
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
  fr_guid_stuff(&writer_guid, &rcvr->src_guid_prefix, &data_submsg->writer_id);
#ifdef VERBOSE_DATA
  printf("  DATA ");
  fr_guid_print(&writer_guid);
  if (little_endian)
    printf(" (LE) ");
  else
    printf(" (BE) ");
  printf(" => 0x%08x  : %d\r\n",
         (unsigned)freertps_htonl(data_submsg->reader_id.u),
         (int)data_submsg->writer_sn.low);
#endif
  // special-case SEDP, since some SEDP broadcasts (e.g., from opensplice
  // sometimes (?)) seem to come with reader_id set to 0
  //fr_entity_id_t reader_id = data_submsg->reader_id;
  //if (data_submsg->writer_id.u == 0xc2030000)
  //  reader_id.u = 0xc7030000;

  int num_matches_found = 0;
  // spin through the readers to see if we care about this data message
  for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.readers);
       it.data; fr_iterator_next(&it))
  {
    struct fr_reader *reader = it.data;
    //printf("      testing reader %08x\n",
    //    freertps_htonl(reader->endpoint.entity_id.u));
    bool match_found = false;
    if (reader->endpoint.reliable)
    {
      for (struct fr_iterator matched_writer_it =
               fr_iterator_begin(reader->matched_writers);
           matched_writer_it.data; fr_iterator_next(&matched_writer_it))
      {
        struct fr_writer_proxy *matched_writer = matched_writer_it.data;
        // todo: something smart
        // check if the writer GUID is equal to the matched-writer GUID
        if (fr_guid_prefix_identical(&rcvr->src_guid_prefix, 
            &matched_writer->remote_writer_guid.prefix) &&
            matched_writer->remote_writer_guid.entity_id.u ==
            data_submsg->writer_id.u)
        {
          match_found = true;
          fr_sequence_number_t msg_sn =
              ((int64_t)data_submsg->writer_sn.high << 32) |
              data_submsg->writer_sn.low;
          if (msg_sn > matched_writer->highest_sequence_number)
            matched_writer->highest_sequence_number = msg_sn;
        }
      }
    }
    else // this is an unreliable reader.
    {
      // have to special-case the SPDP entity ID's, since they come in
      // with any GUID prefix and with either an unknown reader entity ID
      // or the unknown-reader entity ID
      bool spdp_match = (data_submsg->writer_id.u == g_spdp_writer_id.u) &&
          (reader->endpoint.entity_id.u == g_spdp_reader_id.u);
      // check if the data message's destination is our GUID
      bool entity_id_match =
          (reader->endpoint.entity_id.u == data_submsg->reader_id.u);
      /*
      printf("rcvr dst guid prefix: ");
      fr_guid_print_prefix(&rcvr->dst_guid_prefix);
      printf("  participant dst guid prefix: ");
      fr_guid_print_prefix(&g_fr_participant.guid_prefix);
      printf("\n");
      */
      bool dst_prefix_match =
          fr_guid_prefix_identical(&rcvr->dst_guid_prefix,
              &g_fr_participant.guid_prefix);
      //reader->endpoint.entity_id.u == g_fr_entity_id_unknown.u);
      /*
      printf("EID match = %d     DST_PREFIX_MATCH = %d\n",
          entity_id_match ? 1 : 0,
          dst_prefix_match ? 1 : 0);
      */
      if (spdp_match || (entity_id_match && dst_prefix_match))
        match_found = true;
    }
    if (match_found)
    {
      num_matches_found++;
      if (reader->data_rx_cb)
        reader->data_rx_cb(rcvr, submsg, scheme, data);
      if (reader->msg_rx_cb)
        reader->msg_rx_cb(data);
      /*
      if (match->msg_cb)
         match->msg_cb(data);
      */
    }
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
    fr_guid_print(&writer_guid);
    printf("\n");
    printf("    available readers:\n");
    for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.readers);
         it.data; fr_iterator_next(&it))
    {
      struct fr_reader *reader = it.data;
      printf("      %08x\n",
          freertps_htonl(reader->endpoint.entity_id.u));
      /*
      fr_reader_t *match = &g_fr_readers[i];
      printf("      writer = ");
      fr_guid_print(&match->writer_guid);
      printf(" => %08x\n", 
        (unsigned)freertps_htonl(match->reader_entity_id.u));
      */
    }
  }

#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
    /*
    // this was the old match test condition...
    if (sub->writer_id.u == data_submsg->writer_id.u &&
        (sub->reader_id.u == data_submsg->reader_id.u ||
         data_submsg->reader_id.u == g_fr_entity_id_unknown.u))
    */
    // update the max-received sequence number counter
  //FREERTPS_ERROR("  ahh unknown data scheme: 0x%04x\n", (unsigned)scheme);
#endif
}

static void fr_message_rx_data_frag(RX_MSG_ARGS)
{
  // todo
}

#if CURRENTLY_UNUSED_BUT_WILL_NEED_FOR_RELIABLE_READERS_SOMEDAY
static uint32_t g_fr_udp_tx_buf_wpos;
static uint8_t g_fr_udp_tx_buf[1536]; //FR_DISCOVERY_TX_BUFLEN];

static void fr_message_tx_acknack(const struct fr_guid_prefix *guid_prefix,
    const fr_entity_id_t         *reader_id,
    const struct fr_guid        *writer_guid,
    const struct fr_sequence_number_set *set)
{
#ifdef HORRIBLY_BROKEN
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
#endif
#ifdef HORRIBLY_BROKEN
  fr_tx(part->metatraffic_unicast_locator.addr.udp4.addr,
           part->metatraffic_unicast_locator.port,
           (const uint8_t *)msg, payload_len);
#endif
}
#endif
