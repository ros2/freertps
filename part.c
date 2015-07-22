#include <stdlib.h>
#include "freertps/udp.h"
#include "freertps/disco.h"
#include "freertps/part.h"

frudp_part_t *frudp_part_find(const frudp_guid_prefix_t *guid_prefix)
{
  for (int i = 0; i < g_frudp_disco_num_parts; i++)
  {
    frudp_part_t *p = &g_frudp_disco_parts[i]; // shorter
    bool match = true;
    for (int j = 0; match && j < FRUDP_GUID_PREFIX_LEN; j++)
    {
      if (guid_prefix->prefix[j] != p->guid_prefix.prefix[j])
        match = false;
    }
    if (match)
      return p;
  }
  return NULL; // couldn't find it. sorry.
}
