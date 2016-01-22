#ifndef FREERTPS_HC_H
#define FREERTPS_HC_H

#include "types.h"
#include "cache_change.h"

typedef struct history_cache
{
  cache_change_t *cc_rptr, *cc_wptr; // linked-list ftw
} history_cache_t;

// prefix: hc = HistoryCache (as defined in the RTPS spec)
history_cache_t *hc_create();

// these functions are defined in the spec
void hc_add_change(history_cache_t *hc, cache_change_t *cc);
void hc_remove_change(history_cache_t *hc, cache_change_t *cc);
cache_change_t *hc_get(history_cache_t *hc);
seq_num_t hc_max(history_cache_t *hc);
seq_num_t hc_min(history_cache_t *hc);

#endif
