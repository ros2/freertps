#ifndef FREERTPS_HC_H
#define FREERTPS_HC_H

#include "freertps/seq_num.h"
#include "freertps/cache_change.h"
#include "freertps/container.h"

typedef struct fr_history_cache
{
  struct fr_container *changes;
  //fr_cache_change_t *cc_rptr, *cc_wptr; // linked-list ftw
} fr_history_cache_t;

// prefix: hc = HistoryCache (as defined in the RTPS spec)
fr_history_cache_t *fr_hc_create();

// these functions are defined in the spec
void fr_hc_add_change(fr_history_cache_t *hc, fr_cache_change_t *cc);
void fr_hc_remove_change(fr_history_cache_t *hc, fr_cache_change_t *cc);

fr_cache_change_t *fr_hc_get(fr_history_cache_t *hc);
fr_seq_num_t fr_hc_max(fr_history_cache_t *hc);
fr_seq_num_t fr_hc_min(fr_history_cache_t *hc);

#endif
