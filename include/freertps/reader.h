#ifndef FREERTPS_READER_H
#define FREERTPS_READER_H

#include <stdbool.h>
#include "freertps/container.h"
#include "freertps/endpoint.h"
#include "freertps/history_cache.h"
#include "freertps/message.h"
#include "freertps/time.h"

typedef struct fr_reader
{
  struct fr_endpoint endpoint;
  bool expects_inline_qos;
  struct fr_duration heartbeat_response_delay;
  struct fr_duration heartbeat_suppression_duration;
  struct fr_container *matched_writers;
  fr_history_cache_t reader_cache;
  char *topic_name;
  char *type_name;
  ///////////////////////////////////////////////////////////
  fr_message_rx_data_cb_t data_rx_cb; // has some receiver-state debris
  fr_message_rx_cb_t msg_rx_cb; // just gives a pointer to the message
} fr_reader_t;

#define FR_READER_TYPE_BEST_EFFORT 0
#define FR_READER_TYPE_RELIABLE    1
fr_reader_t *fr_reader_create(const char *topic_name, const char *type_name,
    const uint32_t type);

#endif
