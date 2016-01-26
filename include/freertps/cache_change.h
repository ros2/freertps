#ifndef FREERTPS_CACHE_CHANGE_H
#define FREERTPS_CACHE_CHANGE_H

#include <stddef.h>
#include "freertps/guid.h"
#include "freertps/seq_num.h"

typedef struct fr_cache_change
{
  // include the change_kind enum here? not sure it's needed...
  struct fr_guid writer_guid;
  struct fr_sequence_number sequence_number;
  void *data;
  size_t data_len;
} fr_cache_change_t;

#endif 
