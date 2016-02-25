#include <inttypes.h>
#include <string.h>
#include <time.h>
#include "freertps/bswap.h"
#include "freertps/cache_change.h"
#include "freertps/container.h"
#include "freertps/freertps.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include "freertps/participant.h"
#include "freertps/participant_proxy.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"
#include "freertps/udp.h"

////////////////////////////////////////////////////////////////////////////
// local constants
//static fr_participant_t g_fr_spdp_rx_part; // just for rx buffer
static fr_participant_proxy_t g_fr_spdp_participant_proxy; // for rx buffer

const union fr_entity_id g_spdp_writer_id = { .u = 0xc2000100 };
const union fr_entity_id g_spdp_reader_id = { .u = 0xc7000100 };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//#define SPDP_VERBOSE
static struct fr_writer *g_fr_spdp_writer;

static void fr_spdp_timer()
{
  fr_writer_unsent_changes_reset(g_fr_spdp_writer);
}

static void fr_spdp_rx_data(struct fr_receiver *rcvr,
                            const struct fr_submessage *submsg,
                            const uint16_t scheme,
                            const uint8_t *data)
{
#ifdef SPDP_VERBOSE
  FREERTPS_INFO("    fr_spdp_rx_data()\n");
#endif
  if (scheme != FR_SCHEME_PL_CDR_LE)
  {
    FREERTPS_ERROR("expected spdp data to be PL_CDR_LE but found %d...\n",
        scheme);
    return;
  }
  fr_participant_proxy_t *part = &g_fr_spdp_participant_proxy;

  struct fr_parameter_list_item *item = (struct fr_parameter_list_item *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    if (item->pid == FR_PID_SENTINEL)
      break;
    else if (item->pid == FR_PID_PROTOCOL_VERSION)
    {
      part->protocol_version.major = item->value[0];
      part->protocol_version.minor = item->value[1];
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp proto version %d.%d\n",
          part->protocol_version.major,
          part->protocol_version.minor);
#endif
    }
    else if (item->pid == FR_PID_VENDOR_ID)
    {
      part->vendor_id = item->value[1] << 8 | item->value[0];
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp vendor_id 0x%04x = %s\n",
          (unsigned)part->vendor_id,
          fr_vendor_id_string(part->vendor_id));
#endif
    }
    else if (item->pid == FR_PID_DEFAULT_UNICAST_LOCATOR)
    {
      struct fr_locator *loc = (struct fr_locator *)item->value;
      part->default_unicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FR_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp unicast locator udpv4: %s:%d\n",
                      fr_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
    }
    else if (item->pid == FR_PID_DEFAULT_MULTICAST_LOCATOR)
    {
      struct fr_locator *loc = (struct fr_locator *)item->value;
      part->default_multicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FR_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp multicast locator udpv4: %s:%d\n",
                      fr_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
      else
        FREERTPS_INFO("        spdp unknown mcast locator kind: %d\n", (int)loc->kind);
    }
    else if (item->pid == FR_PID_METATRAFFIC_UNICAST_LOCATOR)
    {
      struct fr_locator *loc = (struct fr_locator *)item->value;
      part->metatraffic_unicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FR_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp metatraffic unicast locator udpv4: %s:%d\n",
                      fr_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
      else if (loc->kind == FR_LOCATOR_KIND_UDPV6)
      {
        // ignore ip6 for now...
      }
      else
        FREERTPS_INFO("        spdp unknown metatraffic mcast locator kind: %d\n", (int)loc->kind);
    }
    else if (item->pid == FR_PID_METATRAFFIC_MULTICAST_LOCATOR)
    {
      struct fr_locator *loc = (struct fr_locator *)item->value;
      part->metatraffic_multicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FR_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp metatraffic multicast locator udpv4: %s:%d\n",
                      fr_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
      else if (loc->kind == FR_LOCATOR_KIND_UDPV6)
      {
        // ignore ip6 for now...
      }
      else
        FREERTPS_INFO("        spdp unknown metatraffic mcast locator kind: %d\n", (int)loc->kind);
    }
    else if (item->pid == FR_PID_PARTICIPANT_LEASE_DURATION)
    {
      struct fr_duration *dur = (struct fr_duration *)item->value;
      part->lease_duration = *dur;
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp lease duration: %d.%09d\n",
                    dur->sec, dur->nanosec);
#endif
    }
    else if (item->pid == FR_PID_PARTICIPANT_GUID)
    {
      struct fr_guid *guid = (struct fr_guid *)item->value;
      memcpy(&part->guid_prefix, &guid->prefix, FR_GUID_PREFIX_LEN);

#ifdef SPDP_VERBOSE
      uint8_t *p = guid->prefix.prefix;
      FREERTPS_INFO("      guid 0x%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x\n",
                    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
                    p[8], p[9], p[10], p[11]);
#endif
    }
    else if (item->pid == FR_PID_BUILTIN_ENDPOINT_SET)
    {
      fr_builtin_endpoint_set_t *eps =
          (fr_builtin_endpoint_set_t *)item->value;
      part->available_builtin_endpoints = *eps;
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      builtin endpoints: 0x%08x\n",
                    part->available_builtin_endpoints);
#endif
    }
    else if (item->pid == FR_PID_PROPERTY_LIST)
    {
      // todo
    }
    else if (item->pid & 0x8000)
    {
      // ignore vendor-specific PID's
    }
    else
    {
      FREERTPS_ERROR("      unhandled spdp rx param 0x%x len %d\n",
                     (unsigned)item->pid, item->len);
    }

    // now, advance to next item in list...
    item = (fr_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
  // now that we have stuff the "part" buffer, spin through our
  // participant list and see if we already have this one
  bool found = false;
  for (struct fr_iterator it =
           fr_iterator_begin(g_fr_participant.matched_participants);
       it.data; fr_iterator_next(&it))
  {
    //printf("spdp tx\n");
    struct fr_participant_proxy *p = it.data;
    if (fr_guid_prefix_identical(&p->guid_prefix,
                                 &part->guid_prefix))
    {
#ifdef SPDP_VERBOSE
      printf("found match at participant slot %d\n", i);
#endif
      found = true;
      // TODO: see if anything has changed. update if needed
    }
  }
 
  if (!found)
  {
#ifdef SPDP_VERBOSE
    printf("didn't have this participant already.\n");
#endif
    fr_container_append(g_fr_participant.matched_participants,
        part, sizeof(struct fr_participant_proxy), FR_CFLAGS_NONE);
    fr_sedp_add_builtin_endpoints(&part->guid_prefix);
  }
}

void fr_spdp_init()
{
  FREERTPS_INFO("fr_spdp_init()\r\n");
  struct fr_writer *w = g_fr_spdp_writer = fr_writer_create(
      NULL, NULL, FR_WRITER_TYPE_BEST_EFFORT);
  w->endpoint.entity_id = g_spdp_writer_id;
  w->endpoint.with_key = true;
  fr_participant_add_writer(w);

  // todo: previously reader_eid = g_spdp_reader_id was used to mark the 
  // outbound messages from this writer. We'll need to do something a bit
  // more elegant now

  // craft the outbound SPDP message
  struct fr_guid *guid = fr_malloc(sizeof(struct fr_guid));
  memcpy(guid->prefix.prefix, g_fr_participant.guid_prefix.prefix,
      FR_GUID_PREFIX_LEN);
  guid->entity_id.u = FR_ENTITY_ID_PARTICIPANT;

  struct fr_parameter_list *spdp_data = fr_malloc(350); // some will leak...
  fr_parameter_list_init(spdp_data);
  uint32_t pver = 0x00000102;
  fr_parameter_list_append(spdp_data, FR_PID_PROTOCOL_VERSION, &pver, 4);
  uint32_t vid = FREERTPS_VENDOR_ID;
  fr_parameter_list_append(spdp_data, FR_PID_VENDOR_ID, &vid, 4);
  fr_parameter_list_append(spdp_data, FR_PID_DEFAULT_UNICAST_LOCATOR,
      fr_container_head(g_fr_participant.user_unicast_locators),
      sizeof(struct fr_locator));
  fr_parameter_list_append(spdp_data, FR_PID_DEFAULT_MULTICAST_LOCATOR,
      fr_container_head(g_fr_participant.user_multicast_locators),
      sizeof(struct fr_locator));
  fr_parameter_list_append(spdp_data, FR_PID_METATRAFFIC_UNICAST_LOCATOR,
      fr_container_head(g_fr_participant.builtin_unicast_locators),
      sizeof(struct fr_locator));
  fr_parameter_list_append(spdp_data, FR_PID_METATRAFFIC_MULTICAST_LOCATOR,
      fr_container_head(g_fr_participant.builtin_multicast_locators),
      sizeof(struct fr_locator));
  struct fr_duration lease_duration = { .seconds = 100, .fraction = 0 }; // ?
  fr_parameter_list_append(spdp_data, FR_PID_PARTICIPANT_LEASE_DURATION,
      &lease_duration, sizeof(struct fr_duration));
  fr_parameter_list_append(spdp_data, FR_PID_PARTICIPANT_GUID,
      guid, sizeof(struct fr_guid));
  uint32_t builtin_endpoint_set = 0x3f; // i guess this means we've got em all
  fr_parameter_list_append(spdp_data, FR_PID_BUILTIN_ENDPOINT_SET,
      &builtin_endpoint_set, sizeof(uint32_t));
  fr_parameter_list_append(spdp_data, FR_PID_SENTINEL, NULL, 0);
  //printf("spdp bcast len = %d\n", (int)spdp_data->serialized_len);
  fr_writer_new_change(w,
      spdp_data->serialization, spdp_data->serialized_len,
      guid, sizeof(struct fr_guid));

  struct fr_reader_locator spdp_reader_locator;
  fr_reader_locator_init(&spdp_reader_locator);
  //rl->locator = 
  fr_locator_set_udp4(&spdp_reader_locator.locator, FR_DEFAULT_MCAST_GROUP,
      fr_participant_mcast_builtin_port());
  fr_writer_add_reader_locator(w, &spdp_reader_locator);

  ////////////////////

  struct fr_reader *r = fr_reader_create(
      NULL, NULL, FR_READER_TYPE_BEST_EFFORT);
  r->endpoint.entity_id = g_spdp_reader_id;
  r->data_rx_cb = fr_spdp_rx_data;
  fr_participant_add_reader(r);

  freertps_add_timer(500000, fr_spdp_timer);
}

void fr_spdp_fini()
{
  FREERTPS_INFO("sdp fini\n");
}

