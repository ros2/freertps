#include "freertps/container.h"
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
  w->last_change_sequence_number.high = 0;
  w->last_change_sequence_number.low  = 0;
  // not sure what to use for the default matched-reader array buffer.
  // using 1 for now just to test the array-block linking more heavily
  w->matched_readers = fr_container_create(sizeof(struct fr_reader_proxy), 1);
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
  fr_sequence_number_increment(&w->last_change_sequence_number);
  cc.writer_entity_id.u = w->endpoint.entity_id.u;
  cc.sequence_number = w->last_change_sequence_number;
  cc.data = data;
  cc.data_len = data_len;
  cc.instance_handle = handle;
  cc.instance_handle_len = handle_len;
  fr_history_cache_add_change(&w->writer_cache, &cc);
  return true;
}

