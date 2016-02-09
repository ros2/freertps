#ifndef FREERTPS_HC_H
#define FREERTPS_HC_H

#include "freertps/sequence_number.h"
#include "freertps/cache_change.h"

typedef struct fr_history_cache
{
  struct fr_container *changes;
} fr_history_cache_t;

void fr_history_cache_init(struct fr_history_cache *hc);
struct fr_history_cache *fr_history_cache_create();

void fr_history_cache_add_change(struct fr_history_cache *hc,
                                 struct fr_cache_change *cc);

void fr_history_cache_remove_change(struct fr_history_cache *hc,
                                    struct fr_cache_change *cc);

fr_cache_change_t *fr_hc_get_change(struct fr_history_cache *hc);
fr_sequence_number_t fr_history_cache_max(struct fr_history_cache *hc);
fr_sequence_number_t fr_history_cache_min(struct fr_history_cache *hc);

#endif
