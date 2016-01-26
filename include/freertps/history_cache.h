#ifndef FREERTPS_HC_H
#define FREERTPS_HC_H

#include "freertps/sequence_number.h"
#include "freertps/cache_change.h"
#include "freertps/container.h"

typedef struct fr_history_cache
{
  struct fr_container *changes;
} fr_history_cache_t;

fr_history_cache_t *fr_history_cache_create();

void fr_history_cache_add_change(fr_history_cache_t *hc,
                                 fr_cache_change_t *cc);

void fr_history_cache_remove_change(fr_history_cache_t *hc,
                                    fr_cache_change_t *cc);

fr_cache_change_t *fr_hc_get_change(fr_history_cache_t *hc);
fr_sequence_number_t fr_hc_max(fr_history_cache_t *hc);
fr_sequence_number_t fr_hc_min(fr_history_cache_t *hc);

#endif
