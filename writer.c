#include <stdio.h>
#include <string.h>
#include "freertps/bswap.h"
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include "freertps/message.h"
#include "freertps/participant_proxy.h"
#include "freertps/udp.h"
#include "freertps/writer.h"

static void fr_writer_send(struct fr_writer *writer,
    fr_sequence_number_t seq_num, struct fr_locator *loc,
    const union fr_entity_id eid);
static void fr_writer_send_change(struct fr_writer *writer,
    struct fr_cache_change *cc, struct fr_locator *loc,
    const union fr_entity_id eid);

struct fr_writer *fr_writer_create(
    const char *topic_name,
    const char *type_name,
    const uint32_t type)
{
  struct fr_writer *w = fr_malloc(sizeof(struct fr_writer));
  fr_endpoint_init(&w->endpoint);
  w->push_mode = false;
  w->heartbeat_period = g_fr_duration_zero;
  w->nack_response_delay = g_fr_duration_zero;
  w->nack_suppression_duration = g_fr_duration_zero;
  w->resend_data_period = g_fr_duration_zero;
  w->last_change_sequence_number = 0;
  // not sure what to use for the default reader-proxy buffers
  if (type == FR_WRITER_TYPE_BEST_EFFORT)
  {
    printf("fr_writer_create(best-effort, topic=%s, type=%s)\n",
        topic_name ? topic_name : "(null)",
        type_name ? type_name : "(null)");
    w->endpoint.reliable = false;
    w->reader_locators = 
        fr_container_create(sizeof(struct fr_reader_locator), 5);
    w->matched_readers = NULL;
  }
  else
  {
    printf("fr_writer_create(reliable, topic=%s, type=%s)\n",
        topic_name ? topic_name : "(null)",
        type_name ? type_name : "(null)");
    w->endpoint.reliable = true;
    w->reader_locators = NULL;
    w->matched_readers =
        fr_container_create(sizeof(struct fr_reader_proxy), 5);
  }
  fr_history_cache_init(&w->writer_cache);

  if (topic_name)
  {
    w->topic_name = fr_malloc(strlen(topic_name)+1);
    strcpy(w->topic_name, topic_name);
  }
  else
    w->topic_name = NULL;

  if (type_name)
  {
    w->type_name = fr_malloc(strlen(type_name)+1);
    strcpy(w->type_name, type_name);
  }
  else
    w->type_name = NULL;

  return w;
}

void fr_writer_destroy(fr_writer_t *w)
{
  fr_free(w);
}

bool fr_writer_new_change(struct fr_writer *w, void *data, size_t data_len,
    void *handle, uint32_t handle_len)
{
  struct fr_cache_change cc;
  w->last_change_sequence_number++;
  printf("fr_writer_new_change()  max_sn now = %d\n",
      (int)w->last_change_sequence_number);
  cc.writer_entity_id.u = w->endpoint.entity_id.u;
  cc.sequence_number = w->last_change_sequence_number;
  cc.data = data;
  cc.data_len = data_len;
  cc.instance_handle = handle;
  cc.instance_handle_len = handle_len;
  fr_history_cache_add_change(&w->writer_cache, &cc);
  return true;
}

void fr_writer_blocking_write(struct fr_writer *w, void *data, size_t data_len,
    void *handle, uint32_t handle_len)
{
  struct fr_cache_change cc;
  w->last_change_sequence_number++;
  printf("fr_writer_blocking_write()  max_sn now = %d\n",
      (int)w->last_change_sequence_number);
  cc.writer_entity_id.u = w->endpoint.entity_id.u;
  cc.sequence_number = w->last_change_sequence_number;
  cc.data = data;
  cc.data_len = data_len;
  cc.instance_handle = handle;
  cc.instance_handle_len = handle_len;
  for (struct fr_iterator it = fr_iterator_begin(w->reader_locators);
       it.data; fr_iterator_next(&it))
  {
    struct fr_reader_locator *rl = it.data;
    rl->highest_seq_num_sent = cc.sequence_number;
    fr_writer_send_change(w, &cc, &rl->locator, g_fr_entity_id_unknown);
  }
}

fr_rc_t fr_writer_add_reader_locator(struct fr_writer *w,
    struct fr_reader_locator *reader_locator)
{
  //printf("ep reliable? %s\n", w->endpoint.reliable ? "yes" : "no");
  if (!w->reader_locators)
  {
    printf("woah! tried to add a reader locator to a reliable writer\n");
    return FR_RC_OH_NOES;
  }
  fr_rc_t rc = fr_container_append(w->reader_locators, reader_locator,
      sizeof(struct fr_reader_locator), FR_CFLAGS_NONE);
  return rc;
}

void fr_writer_unsent_changes_reset(struct fr_writer *w)
{
  //printf("unsent changes reset\n");
  for (struct fr_iterator it = fr_iterator_begin(w->reader_locators);
       it.data; fr_iterator_next(&it))
  {
    //printf("spdp tx\n");
    struct fr_reader_locator *rl = it.data;
    rl->highest_seq_num_sent = 0;
  }
}

void fr_writer_send_changes(struct fr_writer *w)
{
  fr_sequence_number_t max_sn = fr_history_cache_max(&w->writer_cache);
  if (!w->endpoint.reliable)
  {
    for (struct fr_iterator it = fr_iterator_begin(w->reader_locators);
         it.data; fr_iterator_next(&it))
    {
      struct fr_reader_locator *rl = it.data;
      while (rl->highest_seq_num_sent < max_sn)
      {
        rl->highest_seq_num_sent++;
        fr_writer_send(w, rl->highest_seq_num_sent, &rl->locator,
            g_fr_entity_id_unknown);
        //printf("sending change %d...\n", (int)rl->highest_seq_num_sent);
      }
    }
  }
  else
  {
    //printf("sending changes to a reliable reader!\n");
    //printf("fr_writer_send_changes(eid=0x%08x) where HC max_sn = %d\n",
    //    (unsigned)freertps_htonl(w->endpoint.entity_id.u), (int)max_sn);
    for (struct fr_iterator it = fr_iterator_begin(w->matched_readers);
         it.data; fr_iterator_next(&it))
    {
      struct fr_reader_proxy *rp = it.data;
      struct fr_participant_proxy *pp =
          fr_participant_proxy_find(&rp->remote_reader_guid.prefix);
      if (!pp)
      {
        printf("woah! couldn't find remote reader prefix ");
        fr_guid_print_prefix(&rp->remote_reader_guid.prefix);
        printf(" in participant-proxy list.\n");
        continue;
      }
      // todo: could be smarter about combining updates into multicast?
      struct fr_locator *loc = NULL;
      if (!w->topic_name)
        loc = &pp->metatraffic_unicast_locator;
      else
        loc = &pp->default_unicast_locator;
      while (rp->highest_seq_num_sent < max_sn)
      {
        rp->highest_seq_num_sent++;
        // look up the locator for this remote reader
        fr_writer_send(w, rp->highest_seq_num_sent, loc,
            rp->remote_reader_guid.entity_id);
        printf("sending change #%d to ", (int)rp->highest_seq_num_sent);
        fr_guid_print(&rp->remote_reader_guid);
        printf("\n");
      }
    }
  }
}

// many/most messages are small. for now, we'll just build our submessage
// stream into this buffer. In the future, as we deal with large messages
// again, we'll have a way to spill over into a 64K buffer (max udp packet)
#define FR_SMALL_MSG_BUF_LEN 1024
static uint8_t fr_writer_small_msg_buf[FR_SMALL_MSG_BUF_LEN] = {0};

static void fr_writer_send(struct fr_writer *writer,
    fr_sequence_number_t seq_num, struct fr_locator *loc,
    const union fr_entity_id eid)
{
  struct fr_cache_change *cc = fr_history_cache_get_change(
      &writer->writer_cache, seq_num);
  if (!cc)
  {
    printf("woah! couldn't find cache change SN #%d\n", (int)seq_num);
    return;
  }
  fr_writer_send_change(writer, cc, loc, eid);
}

void fr_writer_send_change(struct fr_writer *writer,
    struct fr_cache_change *cc, struct fr_locator *loc,
    const union fr_entity_id eid)
{
  if (!cc)
    return;
  printf("sending EID 0x%08x SN #%d length %d to ",
      (unsigned)freertps_htonl(writer->endpoint.entity_id.u),
      (int)cc->sequence_number, (int)cc->data_len);
  fr_locator_print(loc);

  struct fr_message *msg = (struct fr_message *)fr_writer_small_msg_buf;
  fr_message_init(msg);
  struct fr_time t = fr_time_now();
  struct fr_submessage *submsg =
      (struct fr_submessage *)&msg->submessages;
  submsg->header.id = FR_SUBMSG_ID_INFO_TS;
  submsg->header.flags = FR_FLAGS_LITTLE_ENDIAN;
  submsg->header.len = 8;
  memcpy(submsg->contents, &t, 8); // optimize later with direct assign...
  struct fr_submessage_data *data_submsg =
      (struct fr_submessage_data *)&msg->submessages[12];
  data_submsg->header.id = FR_SUBMSG_ID_DATA;
  data_submsg->header.flags = FR_FLAGS_LITTLE_ENDIAN | FR_FLAGS_DATA_PRESENT;
  //data_submsg->header.len = 16 + cc->data_len; // 336;
  data_submsg->extraflags = 0;
  data_submsg->octets_to_inline_qos = 16; // ?
  data_submsg->reader_id = eid; //g_fr_entity_id_unknown;
  data_submsg->writer_id = writer->endpoint.entity_id;
  data_submsg->writer_sn.high = (int32_t)(cc->sequence_number >> 32);
  data_submsg->writer_sn.low = (uint32_t)(cc->sequence_number & 0xffffffff);
  uint8_t *wpos = (uint8_t *)data_submsg->data;
  if (writer->endpoint.with_key)
  {
    data_submsg->header.flags |= FR_FLAGS_INLINE_QOS;
    struct fr_parameter_list_item *item =
        (struct fr_parameter_list_item *)wpos;
    item->pid = FR_PID_KEY_HASH;
    item->len = 16;
    memcpy(item->value, cc->instance_handle, cc->instance_handle_len);
    wpos += 4 + item->len;
    item = (struct fr_parameter_list_item *)wpos;
    item->pid = FR_PID_SENTINEL;
    item->len = 0;
    wpos += 4;
  }
  if (writer->type_name)
  {
    // assume all non-NULL type names mean that we are doing CDR serialization
    fr_encapsulation_scheme_t *scheme = (fr_encapsulation_scheme_t *)wpos;
    scheme->scheme = freertps_htons(FR_SCHEME_CDR_LE);
    scheme->options = 0;
    wpos += 4;
  }
  memcpy(wpos, cc->data, cc->data_len);
  wpos += cc->data_len;
  data_submsg->header.len =
      (uint16_t)(wpos - (uint8_t *)&data_submsg->extraflags);
  // TODO: round up to nearest 32-bit alignment
  if (writer->endpoint.reliable)
  {
    // send heartbeats
    struct fr_submessage_heartbeat *hb_submsg =
        (struct fr_submessage_heartbeat *)wpos;
    hb_submsg->header.id = FR_SUBMSG_ID_HEARTBEAT;
    hb_submsg->header.flags = 0x3; // wtf
    hb_submsg->header.len = 28; // wut
    hb_submsg->reader_id = data_submsg->reader_id;
    hb_submsg->writer_id = data_submsg->writer_id;
    hb_submsg->first_sn.low = 1; // todo: not necessarily true...
    hb_submsg->first_sn.high = 0; // ditto
    hb_submsg->last_sn = data_submsg->writer_sn;
    static int hb_count = 0; // TODO: not this
    hb_submsg->count = hb_count++;
    wpos += 4 + hb_submsg->header.len;
  }

  uint32_t payload_len = (uint32_t)(wpos - (uint8_t *)msg);
  //printf("udp payload: %d bytes\n", (int)payload_len);
  fr_udp_tx(loc->addr.udp4.addr, loc->port,
      (const uint8_t *)msg, payload_len);
}

