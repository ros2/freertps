#include "freertps/writer.h"
#include "freertps/mem.h"

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
  w->last_change_sequence_number = g_fr_sequence_number_unknown;
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

