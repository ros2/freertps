#include "freertps/iterator.h"
#include "freertps/participant.h"
#include "freertps/participant_proxy.h"

struct fr_participant_proxy *
fr_participant_proxy_find(const fr_guid_prefix_t *guid_prefix)
{
  for (struct fr_iterator it =
           fr_iterator_begin(g_fr_participant.matched_participants);
       it.data; fr_iterator_next(&it))
  {
    struct fr_participant_proxy *p = it.data;
    if (fr_guid_prefix_identical(&p->guid_prefix, guid_prefix))
      return p;
  }
  return NULL; // couldn't find it. sorry.
 #ifdef HORRIBLY_BROKEN
    for (int j = 0; match && j < FR_GUID_PREFIX_LEN; j++)
    {
      if (guid_prefix->prefix[j] != p->guid_prefix.prefix[j])
        match = false;
    }
    if (match)
      return p;
  }
#endif
}


