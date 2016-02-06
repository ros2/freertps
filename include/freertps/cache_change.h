#ifndef FREERTPS_CACHE_CHANGE_H
#define FREERTPS_CACHE_CHANGE_H

#include <stddef.h>
#include "freertps/guid.h"
#include "freertps/sequence_number.h"

typedef struct fr_cache_change
{
  // include the change_kind enum here? not sure it's needed...
  union fr_entity_id writer_entity_id;
  fr_sequence_number_t sequence_number;
  void *data;
  size_t data_len;
  void *instance_handle;
  uint32_t instance_handle_len;
} fr_cache_change_t;

#endif 
