#include "freertps/reader.h"
#include "freertps/mem.h"
#include "freertps/writer_proxy.h"

struct fr_reader *fr_reader_create(
    const char *topic_name,
    const char *type_name,
    const uint32_t type)
{
  struct fr_reader *r = fr_malloc(sizeof(struct fr_reader));
  fr_endpoint_init(&r->endpoint);
  r->endpoint.reliable = (type == FR_READER_TYPE_RELIABLE);
  r->expects_inline_qos = false;
  r->heartbeat_response_delay = g_fr_duration_zero;
  r->heartbeat_suppression_duration = g_fr_duration_zero;
  fr_history_cache_init(&r->reader_cache);
  r->matched_writers = fr_container_create(sizeof(struct fr_writer_proxy), 1);
  return r;
}

