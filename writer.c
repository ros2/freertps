#include <stdio.h>
#include <string.h>
#include "freertps/bswap.h"
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include "freertps/message.h"
#include "freertps/writer.h"
#include "freertps/udp.h"

static void fr_writer_send(struct fr_writer *writer,
    fr_sequence_number_t seq_num, struct fr_locator *loc);

struct fr_writer *fr_writer_create(
    const char *topic_name,
    const char *type_name,
    const uint32_t type)
{
  // todo: duplicate topic_name and type_name?
  struct fr_writer *w = fr_malloc(sizeof(struct fr_writer));
  fr_endpoint_init(&w->endpoint);
  w->endpoint.reliable = (type == FR_WRITER_TYPE_RELIABLE);
  w->push_mode = false;
  w->heartbeat_period = g_fr_duration_zero;
  w->nack_response_delay = g_fr_duration_zero;
  w->nack_suppression_duration = g_fr_duration_zero;
  w->resend_data_period = g_fr_duration_zero;
  w->last_change_sequence_number = 0;
  // not sure what to use for the default reader-proxy buffers
  if (type == FR_WRITER_TYPE_BEST_EFFORT)
  {
    printf("fr_writer_create(reliable, topic=%s, type=%s)\n",
        topic_name ? topic_name : "(null)",
        type_name ? type_name : "(null)");
    w->reader_locators = 
        fr_container_create(sizeof(struct fr_reader_locator), 5);
    w->matched_readers = NULL;
  }
  else
  {
    printf("fr_writer_create(best-effort, topic=%s, type=%s)\n",
        topic_name ? topic_name : "(null)",
        type_name ? type_name : "(null)");
    w->reader_locators = NULL;
    w->matched_readers =
        fr_container_create(sizeof(struct fr_reader_proxy), 5);
  }
  fr_history_cache_init(&w->writer_cache);
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
  //printf("fr_writer_send_changes() where max_sn = %d\n", (int)max_sn);
  for (struct fr_iterator it = fr_iterator_begin(w->reader_locators);
       it.data; fr_iterator_next(&it))
  {
    struct fr_reader_locator *rl = it.data;
    while (rl->highest_seq_num_sent < max_sn)
    {
      rl->highest_seq_num_sent++;
      fr_writer_send(w, rl->highest_seq_num_sent, &rl->locator);
      //printf("sending change %d...\n", (int)rl->highest_seq_num_sent);
    }
  }
}

// many/most messages are small. for now, we'll just build our submessage
// stream into this buffer. In the future, as we deal with large messages
// again, we'll have a way to spill over into a 64K buffer (max udp packet)
#define FR_SMALL_MSG_BUF_LEN 1024
static uint8_t fr_writer_small_msg_buf[FR_SMALL_MSG_BUF_LEN];

static void fr_writer_send(struct fr_writer *writer,
    fr_sequence_number_t seq_num, struct fr_locator *loc)
{
  struct fr_cache_change *cc = fr_history_cache_get_change(
      &writer->writer_cache, seq_num);
  if (!cc)
  {
    printf("woah! couldn't find cache change SN #%d\n", (int)seq_num);
    return;
  }
  printf("sending SN #%d length %d to ", (int)seq_num, (int)cc->data_len);
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
  data_submsg->reader_id = g_fr_entity_id_unknown;
  data_submsg->writer_id = writer->endpoint.entity_id;   //g_spdp_writer_id;
  data_submsg->writer_sn.high = (int32_t)(seq_num >> 32);
  data_submsg->writer_sn.low = (uint32_t)(seq_num & 0xffffffff);
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
  memcpy(wpos, cc->data, cc->data_len);
  wpos += cc->data_len;
  data_submsg->header.len =
      (uint16_t)(wpos - (uint8_t *)&data_submsg->extraflags);
  uint32_t payload_len = (uint32_t)(wpos - (uint8_t *)msg);
  printf("udp payload: %d bytes\n", (int)payload_len);
  fr_udp_tx(freertps_htonl(loc->addr.udp4.addr), loc->port,
      (const uint8_t *)msg, payload_len);
  if (writer->endpoint.reliable)
  {
    // send heartbeats
  }
}

