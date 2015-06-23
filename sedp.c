#include "freertps/sedp.h"
#include "freertps/freertps.h"
#include "freertps/udp.h"
#include "freertps/discovery.h"
#include "freertps/qos.h"
#include "freertps/participant.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////
// local constants
static char g_sedp_string_buf[256];
static const frudp_entity_id_t g_sedp_pub_writer_id = { .u = 0xc2030000 };
static const frudp_entity_id_t g_sedp_pub_reader_id = { .u = 0xc7030000 };

////////////////////////////////////////////////////////////////////////////
// local functions
static void frudp_sedp_rx_pub_data(frudp_receiver_state_t *rcvr,
                                   const frudp_submsg_t *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data);
static void frudp_sedp_rx_pub_heartbeat(frudp_receiver_state_t *rcvr,
                                        const frudp_submsg_heartbeat_t *hb);
static void frudp_sedp_bcast();
////////////////////////////////////////////////////////////////////////////

static fr_time_t frudp_sedp_last_bcast;

void frudp_sedp_init()
{
  FREERTPS_INFO("sedp init\n");
  frudp_subscribe(g_sedp_pub_reader_id,
                  g_sedp_pub_writer_id,
                  frudp_sedp_rx_pub_data,
                  frudp_sedp_rx_pub_heartbeat); 
  frudp_subscribe(g_frudp_entity_id_unknown,
                  g_sedp_pub_writer_id,
                  frudp_sedp_rx_pub_data,
                  frudp_sedp_rx_pub_heartbeat); 
}

void frudp_sedp_fini()
{
  FREERTPS_INFO("sedp fini\n");
}

void frudp_sedp_tick()
{
  const fr_time_t t = fr_time_now();
  if (fr_time_diff(&t, &frudp_sedp_last_bcast).seconds >= 1)
  {
    frudp_sedp_bcast();
    frudp_sedp_last_bcast = t;
  }
  //FREERTPS_INFO("sedp tick\n");
}

void frudp_tx_acknack(const frudp_guid_prefix_t *guid_prefix,
                      const frudp_entity_id_t *reader_id,
                      const frudp_entity_id_t *writer_id,
                      const frudp_sequence_number_set_t *set)
{
  static int s_acknack_count = 1;
  // find the participant we are trying to talk to
  frudp_participant_t *part = frudp_participant_find(guid_prefix);
  if (!part)
  {
    FREERTPS_ERROR("tried to acknack an unknown participant\n");
    return; // woah.
  }
  frudp_msg_t *msg = (frudp_msg_t *)g_frudp_discovery_tx_buf;
  frudp_init_msg(msg);
  FREERTPS_INFO("about to tx acknack\n");
  frudp_submsg_t *dst_submsg = (frudp_submsg_t *)&msg->submsgs[0];
  dst_submsg->header.id = FRUDP_SUBMSG_ID_INFO_DEST;
  dst_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  dst_submsg->header.len = 12;
  memcpy(dst_submsg->contents, guid_prefix, FRUDP_GUID_PREFIX_LEN);
  frudp_submsg_t *acknack_submsg = (frudp_submsg_t *)(&msg->submsgs[16]);
  acknack_submsg->header.id = FRUDP_SUBMSG_ID_ACKNACK;
  acknack_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  acknack_submsg->header.len = 24 + (set->num_bits + 31)/32 * 4;
  frudp_submsg_acknack_t *acknack = 
                            (frudp_submsg_acknack_t *)acknack_submsg->contents;
  acknack->reader_id = *reader_id;
  acknack->writer_id = *writer_id;
  int sn_set_len = (set->num_bits + 31) / 32 * 4 + 12;
  memcpy(&acknack->reader_sn_state, set, sn_set_len);
  uint32_t *p_count = (uint32_t *)&acknack->reader_sn_state + sn_set_len / 4;
  *p_count = s_acknack_count++;
  uint8_t *p_next_submsg = (uint8_t *)p_count + 4;
  int payload_len = p_next_submsg - (uint8_t *)msg;
  frudp_tx(part->metatraffic_unicast_locator.addr.udp4.addr,
           part->metatraffic_unicast_locator.port,
           (const uint8_t *)msg, payload_len);
}

static void frudp_sedp_rx_pub_data(frudp_receiver_state_t *rcvr,
                                   const frudp_submsg_t *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data)
{
#ifdef SEDP_VERBOSE
  printf("  sedp_writer data rx\n");
#endif
  if (scheme != FRUDP_ENCAPSULATION_SCHEME_PL_CDR_LE)
  {
    FREERTPS_ERROR("expected sedp data to be PL_CDR_LE. bailing...\n");
    return;
  }
  frudp_parameter_list_item_t *item = (frudp_parameter_list_item_t *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    const frudp_parameterid_t pid = item->pid;
    if (pid == FRUDP_PID_SENTINEL)
      break;
    const uint8_t *pval = item->value;
    if (pid == FRUDP_PID_ENDPOINT_GUID)
    {
      frudp_guid_t *guid = (frudp_guid_t *)pval;
      //memcpy(&part->guid_prefix, &guid->guid_prefix, FRUDP_GUID_PREFIX_LEN);
      uint8_t *p = guid->guid_prefix.prefix;
      printf("    endpoint guid 0x%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x\n",
             p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
             p[8], p[9], p[10], p[11],
             p[12], p[13], p[14], p[15]);

    }
    else if (pid == FRUDP_PID_TOPIC_NAME)
    {
      frudp_rtps_string_t *s = (frudp_rtps_string_t *)pval;
      if (frudp_parse_string(g_sedp_string_buf, sizeof(g_sedp_string_buf), s))
      {
        printf("    topic name: [%s]\n", g_sedp_string_buf);
      }
      else
        FREERTPS_ERROR("couldn't parse topic name of length %d\n", s->len);
    }
    else if (pid == FRUDP_PID_TYPE_NAME)
    {
      frudp_rtps_string_t *s = (frudp_rtps_string_t *)pval;
      if (frudp_parse_string(g_sedp_string_buf, sizeof(g_sedp_string_buf), s))
      {
        printf("    type name: [%s]\n", g_sedp_string_buf);
      }
      else
        FREERTPS_ERROR("couldn't parse type name of length %d\n", s->len);
    }
    else if (pid == FRUDP_PID_RELIABILITY)
    {
      frudp_qos_reliability_t *qos = (frudp_qos_reliability_t *)pval;
      if (qos->kind == FRUDP_QOS_RELIABILITY_KIND_BEST_EFFORT)
      {
        printf("    reliability QoS: [best-effort]\n");
      }
      else if (qos->kind == FRUDP_QOS_RELIABILITY_KIND_RELIABLE)
      {
        printf("    reliability QoS: [reliable]\n");
      }
      else
        FREERTPS_ERROR("unhandled reliability kind: %d\n", qos->kind);
    }
    else if (pid == FRUDP_PID_HISTORY)
    {
      frudp_qos_history_t *qos = (frudp_qos_history_t *)pval;
      if (qos->kind == FRUDP_QOS_HISTORY_KIND_KEEP_LAST)
      {
        printf("    history QoS: [keep last %d]\n", qos->depth);
      }
      else
        FREERTPS_ERROR("unhandled history kind: %d\n", qos->kind);
    }
    else if (pid == FRUDP_PID_TRANSPORT_PRIORITY)
    {
      uint32_t priority = *((uint32_t *)pval);
      printf("    transport priority: %d\n", priority);
    }

    // now, advance to next item in list...
    item = (frudp_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
  // now, we need to ack the packet
  frudp_submsg_data_t *data_submsg = 
                           (frudp_submsg_data_t *)submsg->contents;
  printf("      need to ack nack %d:%d\n",
         data_submsg->writer_sn.high, data_submsg->writer_sn.low);
  frudp_sequence_number_set_t set;
  set.bitmap_base = data_submsg->writer_sn;
  set.bitmap_base.low++;
  set.num_bits = 0;
  frudp_tx_acknack(&rcvr->src_guid_prefix,
                   &g_sedp_pub_reader_id,
                   &g_sedp_pub_writer_id,
                   &set);
}

static void frudp_sedp_rx_pub_heartbeat(frudp_receiver_state_t *rcvr,
                                        const frudp_submsg_heartbeat_t *hb)
{
  printf("      sedp rx heartbeat\n");
  // todo: make this smarter
  frudp_sequence_number_set_32bits_t set;
  set.bitmap_base = hb->first_sn;
  set.num_bits = hb->last_sn.low - hb->first_sn.low;
  set.bitmap_base.low++;
  set.num_bits = 0;
  set.bitmap = 0xffffffff;
  frudp_tx_acknack(&rcvr->src_guid_prefix,
                   &g_sedp_pub_reader_id,
                   &g_sedp_pub_writer_id,
                   (frudp_sequence_number_set_t *)&set);
}

static void frudp_sedp_bcast()
{
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_frudp_discovery_tx_buf);
  fr_time_t t = fr_time_now();
}
