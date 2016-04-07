#include <stdio.h>
#include <stdlib.h>
#include "freertps/udp.h"
#include "freertps/disco.h"
#include "freertps/part.h"
#include "freertps/config.h"
#include "freertps/freertps.h"

//static frudp_part_t g_frudp_participant;
static bool g_frudp_participant_init_complete = false;

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

bool frudp_part_create(void)
{
  if (g_frudp_participant_init_complete)
  {
    printf("woah there partner. "
           "freertps currently only allows one participant.\r\n");
    return false;
  }
  FREERTPS_INFO("frudp_part_create() on domain_id %d\r\n",
                (int)g_frudp_config.domain_id);
  //g_frudp_config.domain_id = domain_id;
  if (!frudp_init_participant_id())
  {
    printf("unable to initialize participant ID\r\n");
    return false;
  }
  //frudp_generic_init();
  //frudp_disco_init();
  g_frudp_participant_init_complete = true;
  printf("prefix: ");
  frudp_print_guid_prefix(&g_frudp_config.guid_prefix);
  printf("\n");
  return true;
}
