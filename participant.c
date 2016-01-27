#include <stdio.h>
#include <stdlib.h>
#include "freertps/udp.h"
#include "freertps/discovery.h"
#include "freertps/participant.h"
#include "freertps/config.h"
#include "freertps/freertps.h"

//static frudp_part_t g_frudp_participant;
static bool g_fr_participant_init_complete = false;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix)
{
  for (int i = 0; i < g_fr_discovery_num_participants; i++)
  {
    fr_participant_t *p = &g_fr_discovery_participants[i]; // shorter
    bool match = true;
    for (int j = 0; match && j < FR_GUID_PREFIX_LEN; j++)
    {
      if (guid_prefix->prefix[j] != p->guid_prefix.prefix[j])
        match = false;
    }
    if (match)
      return p;
  }
  return NULL; // couldn't find it. sorry.
}

bool fr_participant_create()
{
  if (g_fr_participant_init_complete)
  {
    printf("woah there partner. "
           "freertps currently only allows one participant.\r\n");
    return false;
  }
  FREERTPS_INFO("fr_part_create() on domain_id %d\r\n",
                (int)g_fr_config.domain_id);
  //g_fr_config.domain_id = domain_id;
  if (!fr_init_participant_id())
  {
    printf("unable to initialize participant ID\r\n");
    return false;
  }
  //fr_generic_init();
  //fr_discovery_init();
  g_fr_participant_init_complete = true;
  printf("prefix: ");
  fr_guid_print_prefix(&g_fr_config.guid_prefix);
  printf("\n");
  return true;
}
