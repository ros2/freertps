#include <stdio.h>
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/history_cache.h"
#include "freertps/mem.h"

void fr_history_cache_init(struct fr_history_cache *hc)
{
  // not sure what to use for default cache-change array block size
  hc->changes = fr_container_create(sizeof(struct fr_cache_change), 10);
}

struct fr_history_cache *fr_history_cache_create()
{
  struct fr_history_cache *hc = fr_malloc(sizeof(struct fr_history_cache));
  fr_history_cache_init(hc);
  return hc;
}

void fr_history_cache_add_change(struct fr_history_cache *hc,
    struct fr_cache_change *cc)
{
  printf("fr_history_cache_add_change(data_len=%d)\n", (int)cc->data_len);
  fr_container_append(hc->changes,
      cc, sizeof(struct fr_cache_change), FR_CFLAGS_NONE);
}

fr_sequence_number_t fr_history_cache_max(struct fr_history_cache *hc)
{
  fr_sequence_number_t max_sn = FR_SEQUENCE_NUMBER_UNKNOWN;
  for (struct fr_iterator it = fr_iterator_begin(hc->changes);
       it.data; fr_iterator_next(&it))
  {
    struct fr_cache_change *cc = it.data;
    if (cc->sequence_number > max_sn)
      max_sn = cc->sequence_number;
  }
  return max_sn;
}
