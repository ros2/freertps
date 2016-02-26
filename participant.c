#include <stdio.h>
#include <stdlib.h>
#include "freertps/config.h"
#include "freertps/discovery.h"
#include "freertps/freertps.h"
#include "freertps/participant.h"
#include "freertps/participant_proxy.h"
#include "freertps/udp.h"
#include "freertps/iterator.h"
#include "freertps/locator.h"

struct fr_participant g_fr_participant;

static bool g_fr_participant_init_complete = false;

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
  g_fr_participant.matched_participants =
      fr_container_create(sizeof(struct fr_participant_proxy), 10);
  g_fr_participant.writers = fr_container_create(0, 0);
  g_fr_participant.readers = fr_container_create(0, 0);
  g_fr_participant.builtin_unicast_locators =
      fr_container_create(sizeof(struct fr_locator), 2);
  g_fr_participant.builtin_multicast_locators =
      fr_container_create(sizeof(struct fr_locator), 2);
  g_fr_participant.user_unicast_locators =
      fr_container_create(sizeof(struct fr_locator), 2);
  g_fr_participant.user_multicast_locators =
      fr_container_create(sizeof(struct fr_locator), 2);
  g_fr_participant_init_complete = true;
  return true;
}

void fr_participant_fini()
{
  printf("fr_participant_fini()\n");
  fr_container_free(g_fr_participant.writers);
  fr_container_free(g_fr_participant.readers);
  fr_container_free(g_fr_participant.builtin_unicast_locators);
  fr_container_free(g_fr_participant.builtin_multicast_locators);
  fr_container_free(g_fr_participant.user_unicast_locators);
  fr_container_free(g_fr_participant.user_multicast_locators);
  fr_container_free(g_fr_participant.matched_participants);
  g_fr_participant.writers = NULL;
  g_fr_participant.readers = NULL;
  g_fr_participant.builtin_unicast_locators = NULL;
  g_fr_participant.builtin_multicast_locators = NULL;
  g_fr_participant.user_unicast_locators = NULL;
  g_fr_participant.user_multicast_locators = NULL;
  g_fr_participant.matched_participants = NULL;
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
  fr_add_mcast_rx(FR_DEFAULT_MCAST_GROUP,
      fr_participant_mcast_builtin_port(),
      g_fr_participant.builtin_multicast_locators);

  fr_add_mcast_rx(FR_DEFAULT_MCAST_GROUP,
      fr_participant_mcast_user_port(),
      g_fr_participant.user_multicast_locators);

  fr_add_ucast_rx(0, fr_participant_ucast_builtin_port(),
      g_fr_participant.builtin_unicast_locators);

  fr_add_ucast_rx(0, fr_participant_ucast_user_port(),
      g_fr_participant.user_unicast_locators);

  return true;
}

static void print_locators(const char *header, struct fr_container *c)
{
  printf("    %s\n", header);
  for (struct fr_iterator it = fr_iterator_begin(c);
      it.data; fr_iterator_next(&it))
  {
    struct fr_locator *loc = (struct fr_locator *)it.data;
    printf("      ");
    fr_locator_print(loc);
  }
}

void fr_participant_print_locators()
{
  printf("=== partcipant locators ===\n");
  printf("  metatraffic:\n");
  print_locators("unicast:", g_fr_participant.builtin_unicast_locators);
  print_locators("multicast:", g_fr_participant.builtin_multicast_locators);
  printf("  user traffic:\n");
  print_locators("unicast:", g_fr_participant.user_unicast_locators);
  print_locators("multicast:", g_fr_participant.user_multicast_locators);
}

void fr_participant_send_changes()
{
  for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.writers);
      it.data; fr_iterator_next(&it))
  {
    struct fr_writer *w = it.data;
    fr_writer_send_changes(w);
  }
}

struct fr_writer *fr_participant_get_writer(const union fr_entity_id entity_id)
{
  for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.writers);
      it.data; fr_iterator_next(&it))
  {
    struct fr_writer *w = it.data;
    if (w->endpoint.entity_id.u == entity_id.u)
      return w;
  }
  return NULL;
}

struct fr_reader *fr_participant_get_reader(const union fr_entity_id entity_id)
{
  for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.readers);
      it.data; fr_iterator_next(&it))
  {
    struct fr_reader *r = it.data;
    if (r->endpoint.entity_id.u == entity_id.u)
      return r;
  }
  return NULL;
}

