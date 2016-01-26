#ifndef FREERTPS_READER_H
#define FREERTPS_READER_H

#include <stdbool.h>
#include "freertps/container.h"
#include "freertps/endpoint.h"
#include "freertps/history_cache.h"
#include "freertps/time.h"

typedef struct fr_reader
{
  struct fr_endpoint endpoint;
  bool expects_inline_qos;
  struct fr_duration heartbeat_response_delay;
  struct fr_duration heartbeat_suppression_duration;
  fr_history_cache_t hc;
  struct fr_container_t *matched_writers;
} fr_reader_t;

#endif
