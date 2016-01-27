#include <stdio.h>
#include <stdlib.h>
#include "freertps/config.h"
#include "freertps/discovery.h"
#include "freertps/freertps.h"
#include "freertps/participant.h"
#include "freertps/udp.h"

struct fr_participant g_fr_participant;

static bool g_fr_participant_init_complete = false;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix)
{
#ifdef HORRIBLY_BROKEN
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
#endif
  return NULL; // couldn't find it. sorry.
}

bool fr_participant_init()
{
  if (g_fr_participant_init_complete)
  {
    printf("woah there partner. "
           "freertps currently only allows one participant.\r\n");
    return false;
  }
  FREERTPS_INFO("fr_participant_create() on domain_id %d\r\n",
      (int)g_fr_config.domain_id);
  if (!fr_init_participant_id())
  {
    printf("unable to initialize participant ID\r\n");
    return false;
  }
  g_fr_participant_init_complete = true;
  printf("prefix: ");
  fr_guid_print_prefix(&g_fr_config.guid_prefix);
  printf("\n");
  fr_add_mcast_rx(freertps_htonl(FR_DEFAULT_MCAST_GROUP),
                  fr_mcast_builtin_port());
  fr_add_mcast_rx(freertps_htonl(FR_DEFAULT_MCAST_GROUP),
                  fr_mcast_user_port());
  fr_add_ucast_rx(fr_ucast_builtin_port());
  fr_add_ucast_rx(fr_ucast_user_port());
  fr_discovery_init();
  g_fr_participant.default_unicast_locators = NULL;
  g_fr_participant.default_multicast_locators = NULL;
  g_fr_participant.matched_participants = NULL;
  g_fr_participant.writers = NULL;
  g_fr_participant.readers = NULL;
  return true;
}

bool fr_participant_add_writer(struct fr_writer *writer)
{
  printf("fr_participant_add_writer()\r\n");
  return false;
}
