#include <stdio.h>
#include <stdlib.h>
#include "freertps/config.h"
#include "freertps/discovery.h"
#include "freertps/freertps.h"
#include "freertps/participant.h"
#include "freertps/udp.h"
#include "freertps/iterator.h"
#include "freertps/locator.h"

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

uint16_t fr_participant_mcast_builtin_port()
{
  return FR_PORT_PB +
         FR_PORT_DG * g_fr_participant.domain_id +
         FR_PORT_D0;
}

uint16_t fr_participant_ucast_builtin_port()
{
  return FR_PORT_PB +
         FR_PORT_DG * g_fr_participant.domain_id +
         FR_PORT_D1 +
         FR_PORT_PG * g_fr_participant.participant_id;
}

uint16_t fr_participant_mcast_user_port()
{
  return FR_PORT_PB +
         FR_PORT_DG * g_fr_participant.domain_id +
         FR_PORT_D2;
}

uint16_t fr_participant_ucast_user_port()
{
  return FR_PORT_PB +
         FR_PORT_DG * g_fr_participant.domain_id +
         FR_PORT_D3 +
         FR_PORT_PG * g_fr_participant.participant_id;
}

bool fr_participant_init()
{
  // this function assumes that fr_system_init() has already populated
  // the guid_prefix and participant_id fields in g_fr_participant
  if (g_fr_participant_init_complete)
  {
    printf("woah there partner. "
           "freertps currently only allows one participant.\r\n");
    return false;
  }
  g_fr_participant.vendor_id = FREERTPS_VENDOR_ID;
  g_fr_participant.domain_id = 0; // todo: accept domain_id from outside
  FREERTPS_INFO("fr_participant_init() on domain_id %d\r\n",
      (int)g_fr_participant.domain_id);
  g_fr_participant.matched_participants = NULL;
  g_fr_participant.writers = fr_container_create(0, 0);
  g_fr_participant.readers = fr_container_create(0, 0);
  g_fr_participant.default_unicast_locators =
      fr_container_create(sizeof(struct fr_locator), 2);
  g_fr_participant.default_multicast_locators =
      fr_container_create(sizeof(struct fr_locator), 2);
  g_fr_participant_init_complete = true;
  return true;
}

void fr_participant_fini()
{
  printf("fr_participant_fini()\n");
  fr_container_free(g_fr_participant.writers);
  fr_container_free(g_fr_participant.readers);
  fr_container_free(g_fr_participant.default_unicast_locators);
  fr_container_free(g_fr_participant.default_multicast_locators);
  g_fr_participant.writers = NULL;
  g_fr_participant.readers = NULL;
  g_fr_participant.default_unicast_locators = NULL;
  g_fr_participant.default_multicast_locators = NULL;
}

bool fr_participant_add_writer(struct fr_writer *writer)
{
  printf("fr_participant_add_writer()\r\n");
  fr_container_append(g_fr_participant.writers, writer, sizeof(writer),
      FR_CFLAGS_TAKE_OWNERSHIP);
  return false;
}

bool fr_participant_add_reader(struct fr_reader *reader)
{
  printf("fr_participant_add_reader()\r\n");
  fr_container_append(g_fr_participant.readers, reader, sizeof(reader),
      FR_CFLAGS_TAKE_OWNERSHIP);
  return false;
}

bool fr_participant_add_default_locators()
{
  fr_add_mcast_rx(freertps_htonl(FR_DEFAULT_MCAST_GROUP),
      fr_participant_mcast_builtin_port());
  fr_add_mcast_rx(freertps_htonl(FR_DEFAULT_MCAST_GROUP),
      fr_participant_mcast_user_port());
  fr_add_ucast_rx(fr_participant_ucast_builtin_port());
  fr_add_ucast_rx(fr_participant_ucast_user_port());
  return true;
}

void fr_participant_print_locators()
{
  printf("=== partcipant locators ===\n");
  printf("  unicast:\n");
  for (struct fr_iterator it = 
           fr_iterator_begin(g_fr_participant.default_unicast_locators);
       it.data; fr_iterator_next(&it))
  {
    struct fr_locator *loc = (struct fr_locator *)it.data;
    printf("    ");
    locator_print(loc);
  }
  printf("  multicast:\n");
  for (struct fr_iterator it = 
         fr_iterator_begin(g_fr_participant.default_multicast_locators);
       it.data; fr_iterator_next(&it))
  {
    struct fr_locator *loc = (struct fr_locator *)it.data;
    printf("    ");
    locator_print(loc);
  }
}

