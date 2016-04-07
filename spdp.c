#include "freertps/freertps.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"
#include "freertps/udp.h"
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "freertps/bswap.h"
#include "freertps/sub.h"

////////////////////////////////////////////////////////////////////////////
// local constants
static frudp_part_t g_frudp_spdp_rx_part; // just for rx buffer

const frudp_eid_t g_spdp_writer_id = { .u = 0xc2000100 };
const frudp_eid_t g_spdp_reader_id = { .u = 0xc7000100 };

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//#define SPDP_VERBOSE

static void frudp_spdp_rx_data(frudp_receiver_state_t *rcvr,
                               const frudp_submsg_t *submsg,
                               const uint16_t scheme,
                               const uint8_t *data)
{
#ifdef SPDP_VERBOSE
  FREERTPS_INFO("    spdp_rx\n");
#endif
  if (scheme != FRUDP_SCHEME_PL_CDR_LE)
  {
    FREERTPS_ERROR("expected spdp data to be PL_CDR_LE. bailing...\n");
    return;
  }
  frudp_part_t *part = &g_frudp_spdp_rx_part;
  // todo: spin through this param list and save it
  frudp_parameter_list_item_t *item = (frudp_parameter_list_item_t *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    const frudp_parameterid_t pid = item->pid;
    if (pid == FRUDP_PID_SENTINEL)
      break;
    const uint8_t *pval = item->value;
    /*
    FREERTPS_INFO("      unhandled spdp rx param 0x%x len %d\n",
                  (unsigned)pid, item->len);
    */
    if (pid == FRUDP_PID_PROTOCOL_VERSION)
    {
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp proto version 0x%04x\n",
                    *((uint16_t *)(pval)));
#endif
      part->pver = *((frudp_pver_t *)(pval)); // todo: what about alignment?
    }
    else if (pid == FRUDP_PID_VENDOR_ID)
    {
      part->vid = freertps_htons(*((frudp_vid_t *)pval));
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp vendor_id 0x%04x = %s\n",
                    part->vid, frudp_vendor(part->vid));
#endif
    }
    else if (pid == FRUDP_PID_DEFAULT_UNICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->default_unicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp unicast locator udpv4: %s:%d\n",
                      frudp_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
    }
    else if (pid == FRUDP_PID_DEFAULT_MULTICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->default_multicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp multicast locator udpv4: %s:%d\n",
                      frudp_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
      else
        FREERTPS_INFO("        spdp unknown mcast locator kind: %d\n", (int)loc->kind);
    }
    else if (pid == FRUDP_PID_METATRAFFIC_UNICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->metatraffic_unicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp metatraffic unicast locator udpv4: %s:%d\n",
                      frudp_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
      else if (loc->kind == FRUDP_LOCATOR_KIND_UDPV6)
      {
        // ignore ip6 for now...
      }
      else
        FREERTPS_INFO("        spdp unknown metatraffic mcast locator kind: %d\n", (int)loc->kind);
    }
    else if (pid == FRUDP_PID_METATRAFFIC_MULTICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->metatraffic_multicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
#ifdef SPDP_VERBOSE
        FREERTPS_INFO("      spdp metatraffic multicast locator udpv4: %s:%d\n",
                      frudp_ip4_ntoa(loc->addr.udp4.addr),
                      loc->port);
#endif
      }
      else if (loc->kind == FRUDP_LOCATOR_KIND_UDPV6)
      {
        // ignore ip6 for now...
      }
      else
        FREERTPS_INFO("        spdp unknown metatraffic mcast locator kind: %d\n", (int)loc->kind);
    }
    else if (pid == FRUDP_PID_PARTICIPANT_LEASE_DURATION)
    {
      frudp_duration_t *dur = (frudp_duration_t *)pval;
      part->lease_duration = *dur;
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      spdp lease duration: %d.%09d\n",
                    dur->sec, dur->nanosec);
#endif
    }
    else if (pid == FRUDP_PID_PARTICIPANT_GUID)
    {
      frudp_guid_t *guid = (frudp_guid_t *)pval;
      memcpy(&part->guid_prefix, &guid->prefix, FRUDP_GUID_PREFIX_LEN);

#ifdef SPDP_VERBOSE
      uint8_t *p = guid->prefix.prefix;
      FREERTPS_INFO("      guid 0x%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x\n",
                    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
                    p[8], p[9], p[10], p[11]);
#endif
    }
    else if (pid == FRUDP_PID_BUILTIN_ENDPOINT_SET)
    {
      part->builtin_endpoints = *((frudp_builtin_endpoint_set_t *)pval);
#ifdef SPDP_VERBOSE
      FREERTPS_INFO("      builtin endpoints: 0x%08x\n",
                    part->builtin_endpoints);
#endif
    }
    else if (pid == FRUDP_PID_PROPERTY_LIST)
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
    item = (frudp_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
  // now that we have stuff the "part" buffer, spin through our
  // participant list and see if we already have this one
  bool found = false;
  for (int i = 0; !found && i < g_frudp_disco_num_parts; i++)
  {
    frudp_part_t *p = &g_frudp_disco_parts[i];
    if (frudp_guid_prefix_identical(&p->guid_prefix,
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
    if (g_frudp_disco_num_parts < FRUDP_DISCO_MAX_PARTS)
    {
      const int p_idx = g_frudp_disco_num_parts; // save typing
      frudp_part_t *p = &g_frudp_disco_parts[p_idx];
      *p = *part; // save everything plz
      //printf("    saved new participant in slot %d\n", p_idx);
      g_frudp_disco_num_parts++;
      sedp_add_builtin_endpoints(p);
      // push this new participant our SEDP data to speed up the process
      //frudp_send_sedp_
    }
    else
      printf("not enough room to save the new participant.\n");
  }
}

static fr_time_t frudp_spdp_last_bcast;

void frudp_spdp_init(void)
{
  FREERTPS_INFO("sdp init\r\n");
  frudp_spdp_last_bcast.seconds = 0;
  frudp_spdp_last_bcast.fraction = 0;
  frudp_reader_t spdp_reader;
  spdp_reader.writer_guid = g_frudp_guid_unknown;
  spdp_reader.reader_eid = g_spdp_reader_id;
  spdp_reader.max_rx_sn.low = 0;
  spdp_reader.max_rx_sn.high = 0;
  spdp_reader.data_cb = frudp_spdp_rx_data;
  spdp_reader.msg_cb = NULL;
  spdp_reader.reliable = false;
  frudp_add_reader(&spdp_reader);
  /*
  frudp_subscribe(g_frudp_entity_id_unknown,
                  g_spdp_writer_id,
                  frudp_spdp_rx_data,
                  NULL);
  */
}

void frudp_spdp_start(void)
{
  frudp_spdp_tick();
}

void frudp_spdp_fini(void)
{
  FREERTPS_INFO("sdp fini\n");
}

// todo: this will all eventually be factored somewhere else. for now,
// just work through what it takes to send messages

// todo: consolidate spdp and sedp into a 'discovery' module

/*
uint16_t frudp_append_submsg(frudp_msg_t *msg, const uint16_t msg_wpos,
                             const frudp_submsg_t * const submsg)
{
  frudp_submsg_t *s = (frudp_submsg_t *)&msg->submsgs[msg_wpos];
  memcpy(s, submsg, submsg->header.len);
  return msg_wpos + submsg->header.len;
}
*/

#define PLIST_ADVANCE(list_item) \
          do { \
            list_item = (frudp_parameter_list_item_t *) \
                        (((uint8_t *)list_item) + 4 + list_item->len); \
          } while (0)
  //param_list = (frudp_parameter_list_item_t *)(((uint8_t *)param_list) + 4 + param_list->len);


void frudp_spdp_bcast(void)
{
  //FREERTPS_INFO("spdp bcast\n");
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_frudp_disco_tx_buf);
  fr_time_t t = fr_time_now();
  uint16_t submsg_wpos = 0;

  frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
  ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
  ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  ts_submsg->header.len = 8;
  memcpy(ts_submsg->contents, &t, 8);
  submsg_wpos += 4 + 8;

/*
  frudp_submsg_t *data_submsg =
*/
  frudp_submsg_data_t *data_submsg = (frudp_submsg_data_t *)&msg->submsgs[submsg_wpos];
  //(frudp_submsg_data_t *)data_submsg->contents;
  data_submsg->header.id = FRUDP_SUBMSG_ID_DATA;
  data_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN |
                              FRUDP_FLAGS_INLINE_QOS    |
                              FRUDP_FLAGS_DATA_PRESENT  ;
  data_submsg->header.len = 336; // need to compute this dynamically?
  data_submsg->extraflags = 0;
  data_submsg->octets_to_inline_qos = 16; // ?
  data_submsg->reader_id = g_frudp_eid_unknown;
  data_submsg->writer_id = g_spdp_writer_id;
  data_submsg->writer_sn.high = 0;
  //static uint32_t bcast_count = 0;
  data_submsg->writer_sn.low = 1; //++bcast_count;
  /////////////////////////////////////////////////////////////
  frudp_parameter_list_item_t *inline_qos_param =
    (frudp_parameter_list_item_t *)(((uint8_t *)data_submsg) +
                                    sizeof(frudp_submsg_data_t));
  inline_qos_param->pid = FRUDP_PID_KEY_HASH;
  inline_qos_param->len = 16;
  memcpy(inline_qos_param->value, &g_frudp_config.guid_prefix, 12);
  // now i don't know what i'm doing
  inline_qos_param->value[12] = 0;
  inline_qos_param->value[13] = 0;
  inline_qos_param->value[14] = 1;
  inline_qos_param->value[15] = 0xc1;
  PLIST_ADVANCE(inline_qos_param);

  inline_qos_param->pid = FRUDP_PID_SENTINEL;
  inline_qos_param->len = 0;
  /////////////////////////////////////////////////////////////
  frudp_encapsulation_scheme_t *scheme =
    (frudp_encapsulation_scheme_t *)(((uint8_t *)inline_qos_param) + 4);
  scheme->scheme = freertps_htons(FRUDP_SCHEME_PL_CDR_LE);
  scheme->options = 0;
  /////////////////////////////////////////////////////////////
  frudp_parameter_list_item_t *param_list =
    (frudp_parameter_list_item_t *)(((uint8_t *)scheme) + sizeof(*scheme));
  param_list->pid = FRUDP_PID_PROTOCOL_VERSION;
  param_list->len = 4;
  param_list->value[0] = 2;
  param_list->value[1] = 1;
  param_list->value[2] = param_list->value[3] = 0; // pad to 4-byte boundary
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_VENDOR_ID;
  param_list->len = 4;
  param_list->value[0] = (FREERTPS_VENDOR_ID >> 8) & 0xff;
  param_list->value[1] = FREERTPS_VENDOR_ID & 0xff;
  param_list->value[2] = param_list->value[3] = 0; // pad to 4-byte boundary
  /////////////////////////////////////////////////////////////
  frudp_locator_t *loc = NULL;
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_DEFAULT_UNICAST_LOCATOR;
  param_list->len = sizeof(frudp_locator_t); // aka 24, minus jack bauer
  loc = (frudp_locator_t *)param_list->value;
  loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
  loc->port = frudp_ucast_user_port();
  memset(loc->addr.udp4.zeros, 0, 12);
  loc->addr.udp4.addr = g_frudp_config.unicast_addr;
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_DEFAULT_MULTICAST_LOCATOR;
  param_list->len = sizeof(frudp_locator_t);
  loc = (frudp_locator_t *)param_list->value;
  loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
  loc->port = frudp_mcast_user_port();
  memset(loc->addr.udp4.zeros, 0, 12);
  loc->addr.udp4.addr = freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP);
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_METATRAFFIC_UNICAST_LOCATOR;
  param_list->len = sizeof(frudp_locator_t); // aka 24, minus jack bauer
  loc = (frudp_locator_t *)param_list->value;
  loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
  loc->port = frudp_ucast_builtin_port();
  memset(loc->addr.udp4.zeros, 0, 12);
  loc->addr.udp4.addr = g_frudp_config.unicast_addr;
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_METATRAFFIC_MULTICAST_LOCATOR;
  param_list->len = sizeof(frudp_locator_t);
  loc = (frudp_locator_t *)param_list->value;
  loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
  loc->port = frudp_mcast_builtin_port();
  memset(loc->addr.udp4.zeros, 0, 12);
  loc->addr.udp4.addr = freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP);
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_PARTICIPANT_LEASE_DURATION;
  param_list->len = 8;
  frudp_duration_t *duration = (frudp_duration_t *)param_list->value;
  duration->sec = 100;
  duration->nanosec = 0;
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_PARTICIPANT_GUID;
  param_list->len = 16;
  frudp_guid_t *guid = (frudp_guid_t *)param_list->value;
  memcpy(&guid->prefix, &g_frudp_config.guid_prefix,
         sizeof(frudp_guid_prefix_t));
  guid->eid.s.key[0] = 0;
  guid->eid.s.key[1] = 0;
  guid->eid.s.key[2] = 1;
  guid->eid.s.kind = 0xc1; // wtf
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_BUILTIN_ENDPOINT_SET;
  param_list->len = 4;
  uint32_t endpoint_set = 0x3f; //b; // 0x3f;
  memcpy(param_list->value, &endpoint_set, 4);
  /////////////////////////////////////////////////////////////
  PLIST_ADVANCE(param_list);
  param_list->pid = FRUDP_PID_SENTINEL;
  param_list->len = 0;
  //data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
  PLIST_ADVANCE(param_list);
  data_submsg->header.len = param_list->value - 4 - (uint8_t *)&data_submsg->extraflags;
  frudp_submsg_t *next_submsg_ptr = (frudp_submsg_t *)param_list;
  /////////////////////////////////////////////////////////////
  /*
  ts_submsg = (frudp_submsg_t *)param_list;
  ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
  ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  ts_submsg->header.len = 8;
  memcpy(ts_submsg->contents, &t, 8);
  uint8_t *next_submsg_ptr = ((uint8_t *)param_list) + 4 + 8;
  */

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  //data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
  //printf("len = %d\n", data_submsg->header.len);
  /////////////////////////////////////////////////////////////
  //int payload_len = ((uint8_t *)param_list) - ((uint8_t *)msg->submsgs);
  //int payload_len = ((uint8_t *)next_submsg_ptr) - ((uint8_t *)msg->submsgs);
  int payload_len = ((uint8_t *)next_submsg_ptr) - ((uint8_t *)msg);
  if (!frudp_tx(freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP), 
        frudp_mcast_builtin_port(),
        (const uint8_t *)msg, payload_len))
    FREERTPS_ERROR("couldn't transmit SPDP broadcast message\r\n");
}

void frudp_spdp_tick(void)
{
  const fr_time_t t = fr_time_now();
  if (fr_time_diff(&t, &frudp_spdp_last_bcast).seconds >= 1) // every second
  //if (fr_time_diff(&t, &frudp_spdp_last_bcast).fraction >= 1000000000) // every second
  {
    //printf("spdp bcast\r\n");
    frudp_spdp_bcast();
    frudp_spdp_last_bcast = t;
    //printf("%d participants known\n", (int)g_frudp_discovery_num_participants);
  }
}
