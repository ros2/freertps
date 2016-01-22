#ifndef READER_H
#define READER_H

#include "freertps/history_cache.h"

typedef struct reader
{
  // function pointers to anything?
  history_cache_t hc;
} reader_t;

#endif
