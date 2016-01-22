#ifndef FREERTPS_READER_H
#define FREERTPS_READER_H

#include "freertps/history_cache.h"

typedef struct fr_reader
{
  // function pointers to anything?
  fr_history_cache_t hc;
} fr_reader_t;

#endif
