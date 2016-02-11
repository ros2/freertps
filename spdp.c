#include <inttypes.h>
#include <string.h>
#include <time.h>
#include "freertps/bswap.h"
#include "freertps/cache_change.h"
#include "freertps/container.h"
#include "freertps/freertps.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
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
    FREERTPS_ERROR("expected spdp data to be PL_CDR_LE. bailing...\n");
    return;
  }
  fr_participant_proxy_t *part = &g_fr_spdp_participant_proxy;

#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
  // todo: spin through this param list and save it
  fr_parameter_list_item_t *item = (fr_parameter_list_item_t *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    const fr_parameterid_t pid = item->pid;
    if (pid == FR_PID_SENTINEL)
      break;
    const uint8_t *pval = item->value;
    /*
    FREERTPS_INFO("      unhandled spdp rx param 0x%x len %d\n",
                  (unsigned)pid, item->len);
    */
    if (pid == FR_PID_PROTOCOL_VERSION)
    {
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp proto version 0x%04x\n",
                    *((uint16_t *)(pval)));
#endif
      part->pver = *((fr_pver_t *)(pval)); // todo: what about alignment?
    }
    else if (pid == FR_PID_VENDOR_ID)
    {
      part->vid = freertps_htons(*((fr_vid_t *)pval));
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp vendor_id 0x%04x = %s\n",
                    part->vid, fr_vendor(part->vid));
#endif
    }
    else if (pid == FR_PID_DEFAULT_UNICAST_LOCATOR)
    {
      fr_locator_t *loc = (fr_locator_t *)pval;
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
    else if (pid == FR_PID_DEFAULT_MULTICAST_LOCATOR)
    {
      fr_locator_t *loc = (fr_locator_t *)pval;
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
    else if (pid == FR_PID_METATRAFFIC_UNICAST_LOCATOR)
    {
      fr_locator_t *loc = (fr_locator_t *)pval;
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
    else if (pid == FR_PID_METATRAFFIC_MULTICAST_LOCATOR)
    {
      fr_locator_t *loc = (fr_locator_t *)pval;
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
    else if (pid == FR_PID_PARTICIPANT_LEASE_DURATION)
    {
      fr_duration_t *dur = (fr_duration_t *)pval;
      part->lease_duration = *dur;
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp lease duration: %d.%09d\n",
                    dur->sec, dur->nanosec);
#endif
    }
    else if (pid == FR_PID_PARTICIPANT_GUID)
    {
      fr_guid_t *guid = (fr_guid_t *)pval;
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
    else if (pid == FR_PID_BUILTIN_ENDPOINT_SET)
    {
      part->builtin_endpoints = *((fr_builtin_endpoint_set_t *)pval);
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      builtin endpoints: 0x%08x\n",
                    part->builtin_endpoints);
#endif
    }
    else if (pid == FR_PID_PROPERTY_LIST)
    {
      // todo
    }
    else if (pid & 0x8000)
    {
      // ignore vendor-specific PID's
    }
    else
    {
      FREERTPS_ERROR("      unhandled spdp rx param 0x%x len %d\n",
                     (unsigned)pid, item->len);
    }

    // now, advance to next item in list...
    item = (fr_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
  // now that we have stuff the "part" buffer, spin through our
  // participant list and see if we already have this one
  bool found = false;
  for (int i = 0; !found && i < g_fr_disco_num_parts; i++)
  {
    fr_participant_t *p = &g_fr_disco_participants[i];
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
    if (g_fr_disco_num_parts < FR_DISCO_MAX_PARTS)
    {
      const int p_idx = g_fr_disco_num_parts; // save typing
      fr_participant_t *p = &g_fr_disco_participants[p_idx];
      *p = *part; // save everything plz
      //printf("    saved new participant in slot %d\n", p_idx);
      g_fr_disco_num_parts++;
      sedp_add_builtin_endpoints(p);
      // push this new participant our SEDP data to speed up the process
      //fr_send_sedp_
    }
    else
      printf("not enough room to save the new participant.\n");
  }
#endif
}

//static fr_time_t fr_spdp_last_bcast;

void fr_spdp_init()
{
  FREERTPS_INFO("fr_spdp_init()\r\n");
  //fr_spdp_last_bcast.seconds = 0;
  //fr_spdp_last_bcast.fraction = 0;
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

  /*
#ifdef BROKEN
  fr_reader_t spdp_reader;
  spdp_reader.writer_guid = g_fr_guid_unknown;
  spdp_reader.reader_eid = g_spdp_reader_id;
  spdp_reader.max_rx_sn.low = 0;
  spdp_reader.max_rx_sn.high = 0;
  spdp_reader.data_cb = fr_spdp_rx_data;
  spdp_reader.msg_cb = NULL;
  fr_add_reader(&spdp_reader);
#endif
  */
  /*
  frudp_subscribe(g_frudp_entity_id_unknown,
                  g_spdp_writer_id,
                  frudp_spdp_rx_data,
                  NULL);
  */
}

void fr_spdp_fini()
{
  FREERTPS_INFO("sdp fini\n");
}

void fr_spdp_tick()
{
  return;
#ifdef HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
  const fr_time_t t = fr_time_now();
  if (fr_time_diff(&t, &fr_spdp_last_bcast).seconds >= 1) // every second
  //if (fr_time_diff(&t, &frudp_spdp_last_bcast).fraction >= 1000000000) // every second
  {
    //printf("spdp bcast\r\n");
    fr_spdp_bcast();
    fr_spdp_last_bcast = t;
    //printf("%d participants known\n", (int)g_frudp_discovery_num_participants);
  }
#endif
}
