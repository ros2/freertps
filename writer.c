#include <stdio.h>
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include "freertps/writer.h"

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
  for (struct fr_iterator it = fr_iterator_begin(w->reader_locators);
       it.data; fr_iterator_next(&it))
  {
    //printf("spdp tx\n");
    struct fr_reader_locator *rl = it.data;
    rl->highest_seq_num_sent = FR_SEQUENCE_NUMBER_UNKNOWN;
  }
}

void fr_writer_send_changes(struct fr_writer *w)
{
  for (struct fr_iterator it = fr_iterator_begin(w->reader_locators);
       it.data; fr_iterator_next(&it))
  {
    struct fr_reader_locator *rl = it.data;
    // TODO: more hacking here...
  }

}
