#include "freertps/udp.h"
#include "freertps/discovery.h"
#include "freertps/participant.h"

frudp_participant_t *frudp_participant_find(frudp_guid_prefix_t *guid_prefix)
{
  for (int i = 0; i < g_frudp_discovery_num_participants; i++)
  {
    frudp_participant_t *p = &g_frudp_discovery_participants[i]; // shorter
    for (int j = 0; j < FRUDP_GUID_PREFIX_LEN; j++)
    {
      //if (guid_prefix->prefix[j] != p->guid_prefix.prefix[j])
    }
  }
  return NULL; // couldn't find it. sorry.
}
