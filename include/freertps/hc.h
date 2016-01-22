#ifndef HC_H
#define HC_H

#include "udp.h"

typedef struct history_cache
{

} history_cache_t;

// hc = HistoryCache, as defined in the RTPS spec
history_cache_t *hc_create();

// these functions are defined in the spec
void hc_add_change(history_cache_t *hc, cache_change_t *cc);
void hc_remove_change(history_cache_t *hc, cache_change_t *cc);
cache_change_t *hc_get(history_cache_t *hc);
seqnum_t hc_max(history_cache_t *hc);
seqnum_t hc_min(history_cache_t *hc);

#endif
