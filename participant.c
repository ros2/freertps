#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertps/config.h"
#include "freertps/freertps.h"
#include "freertps/iterator.h"
#include "freertps/locator.h"
#include "freertps/mem.h"
#include "freertps/participant.h"
#include "freertps/participant_proxy.h"
#include "freertps/qos.h"
#include "freertps/sedp.h"
#include "freertps/spdp.h"
#include "freertps/udp.h"

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
  if (!writer->topic_name || !writer->type_name)
  {
    printf("  not sending SEDP message for entity ID 0x%08x\n",
        freertps_htonl(writer->endpoint.entity_id.u));
    return true; // no need
  }
  union fr_entity_id ww_eid = { .u = FR_EID_WRITER_WRITER };
  struct fr_writer *writer_writer = fr_participant_get_writer(ww_eid);
  if (!writer_writer)
  {
    printf("BOGUS! probably participant is not correctly initialized!\n");
    return false;
  }
  const int topic_len = strlen(writer->topic_name);
  const int type_len = strlen(writer->type_name);
  // todo: figure out how long this allocation should be
  struct fr_parameter_list *sedp_data = fr_malloc(topic_len + type_len + 300);
  fr_parameter_list_init(sedp_data);
  uint32_t pver = FR_PROTOCOL_VERSION_MAJOR | (FR_PROTOCOL_VERSION_MINOR << 8);
  fr_parameter_list_append(sedp_data, FR_PID_PROTOCOL_VERSION, &pver, 4);
  uint32_t vid = FREERTPS_VENDOR_ID;
  fr_parameter_list_append(sedp_data, FR_PID_VENDOR_ID, &vid, 4);
  struct fr_guid *ep_guid = fr_malloc(16);
  memcpy(ep_guid->prefix.prefix, &g_fr_participant.guid_prefix.prefix,
      FR_GUID_PREFIX_LEN);
  ep_guid->entity_id.u = writer->endpoint.entity_id.u;
  fr_parameter_list_append(sedp_data, FR_PID_ENDPOINT_GUID, ep_guid, 16);
  fr_parameter_list_append_string(sedp_data, FR_PID_TOPIC_NAME,
      writer->topic_name);
  fr_parameter_list_append_string(sedp_data, FR_PID_TYPE_NAME,
      writer->type_name);
  // TODO: branch based on reliability type
  struct fr_qos_reliability reliability;
  reliability.kind = FR_QOS_RELIABILITY_KIND_BEST_EFFORT;
  reliability.max_blocking_time.seconds = 0;
  reliability.max_blocking_time.fraction = 0x19999999;
  fr_parameter_list_append(sedp_data, FR_PID_RELIABILITY, &reliability, 12);
  fr_parameter_list_append(sedp_data, FR_PID_SENTINEL, NULL, 0);
  fr_writer_new_change(writer_writer,
      sedp_data->serialization, sedp_data->serialized_len, ep_guid, 16);

  return true;
}

bool fr_participant_add_reader(struct fr_reader *reader)
{
  printf("fr_participant_add_reader()\r\n");
  fr_container_append(g_fr_participant.readers, reader, sizeof(reader),
      FR_CFLAGS_TAKE_OWNERSHIP);
  if (!reader->topic_name || !reader->type_name)
  {
    printf("  not sending SEDP message for entity ID 0x%08x\n",
        freertps_htonl(reader->endpoint.entity_id.u));
    return true; // no need
  }
  union fr_entity_id rw_eid = { .u = FR_EID_READER_WRITER };
  struct fr_writer *reader_writer = fr_participant_get_writer(rw_eid);
  if (!reader_writer)
  {
    printf("BOGUS! probably participant is not correctly initialized!\n");
    return false;
  }
  const int topic_len = strlen(reader->topic_name);
  const int type_len = strlen(reader->type_name);
  // todo: figure out how long this allocation should be
  struct fr_parameter_list *sedp_data = fr_malloc(topic_len + type_len + 300);
  fr_parameter_list_init(sedp_data);
  uint32_t pver = FR_PROTOCOL_VERSION_MAJOR | (FR_PROTOCOL_VERSION_MINOR << 8);
  fr_parameter_list_append(sedp_data, FR_PID_PROTOCOL_VERSION, &pver, 4);
  uint32_t vid = FREERTPS_VENDOR_ID;
  fr_parameter_list_append(sedp_data, FR_PID_VENDOR_ID, &vid, 4);
  struct fr_guid *ep_guid = fr_malloc(16);
  memcpy(ep_guid->prefix.prefix, &g_fr_participant.guid_prefix.prefix,
      FR_GUID_PREFIX_LEN);
  ep_guid->entity_id.u = reader->endpoint.entity_id.u;
  fr_parameter_list_append(sedp_data, FR_PID_ENDPOINT_GUID, ep_guid, 16);
  fr_parameter_list_append_string(sedp_data, FR_PID_TOPIC_NAME,
      reader->topic_name);
  fr_parameter_list_append_string(sedp_data, FR_PID_TYPE_NAME,
      reader->type_name);
  // TODO: branch based on reliability type
  struct fr_qos_reliability reliability;
  reliability.kind = FR_QOS_RELIABILITY_KIND_BEST_EFFORT;
  reliability.max_blocking_time.seconds = 0;
  reliability.max_blocking_time.fraction = 0x19999999;
  fr_parameter_list_append(sedp_data, FR_PID_RELIABILITY, &reliability, 12);
  fr_parameter_list_append(sedp_data, FR_PID_SENTINEL, NULL, 0);
  fr_writer_new_change(reader_writer,
      sedp_data->serialization, sedp_data->serialized_len, ep_guid, 16);
  return true;
}

bool fr_participant_add_default_locators()
{
  printf("fr_participant_add_default_locators()\n");
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

void fr_participant_discovery_init()
{
  fr_spdp_init();
  fr_sedp_init();
}

void fr_participant_discovery_fini()
{
  fr_spdp_fini();
  fr_sedp_fini();
}

