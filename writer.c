#include <stdio.h>
#include <string.h>
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include "freertps/message.h"
#include "freertps/writer.h"

static void fr_writer_send(fr_sequence_number_t seq_num,
    struct fr_locator *loc);

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
      fr_writer_send(rl->highest_seq_num_sent, &rl->locator);
      //printf("sending change %d...\n", (int)rl->highest_seq_num_sent);
    }
  }
}

// many/most messages are small. for now, we'll just build our submessage
// stream into this buffer. In the future, as we deal with large messages
// again, we'll have a way to spill over into a 64K buffer (max udp packet)
#define FR_SMALL_MSG_BUF_LEN 1024
static uint8_t fr_writer_small_msg_buf[FR_SMALL_MSG_BUF_LEN];

static void fr_writer_send(fr_sequence_number_t seq_num,
    struct fr_locator *loc)
{
  printf("sending SN %d to ", (int)seq_num);
  fr_locator_print(loc);

  struct fr_message *msg = (struct fr_message *)fr_writer_small_msg_buf;
  fr_message_init(msg);
  struct fr_time t = fr_time_now();
  uint16_t submsg_wpos = 0;
  struct fr_submessage *ts_submsg =
      (struct fr_submessage *)&msg->submessages[submsg_wpos];
  ts_submsg->header.id = FR_SUBMSG_ID_INFO_TS;
  ts_submsg->header.flags = FR_FLAGS_LITTLE_ENDIAN;
  ts_submsg->header.len = 8;
  memcpy(ts_submsg->contents, &t, 8); // optimize later with direct assign...
  submsg_wpos += 4 + 8;
  struct fr_submessage_data *data_submsg = (fr_submsg_data_t *)&msg->submsgs[submsg_wpos];
  // todo: continue copy-pasting here from the spdp broadcast tx function...
}
