#include "freertps/sedp.h"
#include "freertps/freertps.h"
#include "freertps/udp.h"
#include "freertps/discovery.h"
#include "freertps/qos.h"
#include "freertps/participant.h"
#include "freertps/subscription.h"
#include "freertps/publisher.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////
// local constants
static char g_sedp_string_buf[256];
static const frudp_entity_id_t g_sedp_pub_writer_id = { .u = 0xc2030000 };
static const frudp_entity_id_t g_sedp_pub_reader_id = { .u = 0xc7030000 };
static const frudp_entity_id_t g_sedp_sub_writer_id = { .u = 0xc2040000 };
static const frudp_entity_id_t g_sedp_sub_reader_id = { .u = 0xc7040000 };

////////////////////////////////////////////////////////////////////////////
// local functions
static void frudp_sedp_rx_pub_data(frudp_receiver_state_t *rcvr,
                                   const frudp_submsg_t *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data);
static void frudp_sedp_bcast();
////////////////////////////////////////////////////////////////////////////
// static globals
static fr_time_t frudp_sedp_last_bcast;
frudp_publisher_t *g_sedp_subscription_pub = NULL;
// todo: an option to generate SEDP messages on-the-fly as requested,
// rather than buffering them in precious SRAM
#define SEDP_MSG_BUF_LEN 1024
// save typing...
#define MAX_SUBS FRUDP_MAX_SUBSCRIPTIONS
//static uint8_t frudp_pub_sample_t[SEDP_MSG_BUF_LEN];
static uint8_t g_sedp_sub_writer_data_buf[SEDP_MSG_BUF_LEN * MAX_SUBS];
static frudp_submsg_data_t *g_sedp_sub_writer_data_submsgs[MAX_SUBS];
//sizeof [MAX_SUBS][SEDP_MSG_BUF_LEN];
static uint8_t g_sedp_sub_buf[SEDP_MSG_BUF_LEN];

void frudp_sedp_init()
{
  FREERTPS_INFO("sedp init\n");
  for (int i = 0; i < MAX_SUBS; i++)
  {
    frudp_submsg_data_t *d = (frudp_submsg_data_t *)&g_sedp_sub_writer_data_buf[i * SEDP_MSG_BUF_LEN];
    g_sedp_sub_writer_data_submsgs[i] = d;
    d->writer_sn = g_frudp_sequence_number_unknown;
  }
  g_sedp_subscription_pub = frudp_create_publisher(NULL, // no topic name
                                                   NULL, // no type name
                                                   g_sedp_sub_writer_id,
                                                   g_sedp_sub_writer_data_submsgs,
                                                   FRUDP_MAX_SUBSCRIPTIONS);
  frudp_subscribe(g_sedp_pub_reader_id,
                  g_sedp_pub_writer_id,
                  frudp_sedp_rx_pub_data,
                  NULL);
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
#ifdef SEDP_VERBOSE
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
#endif
    }
    else if (pid == FRUDP_PID_TOPIC_NAME)
    {
      frudp_rtps_string_t *s = (frudp_rtps_string_t *)pval;
      if (frudp_parse_string(g_sedp_string_buf, sizeof(g_sedp_string_buf), s))
      {
#ifdef SEDP_VERBOSE
        printf("    topic name: [%s]\n", g_sedp_string_buf);
#endif
      }
      else
        FREERTPS_ERROR("couldn't parse topic name of length %d\n", s->len);
    }
    else if (pid == FRUDP_PID_TYPE_NAME)
    {
      frudp_rtps_string_t *s = (frudp_rtps_string_t *)pval;
      if (frudp_parse_string(g_sedp_string_buf, sizeof(g_sedp_string_buf), s))
      {
#ifdef SEDP_VERBOSE
        printf("    type name: [%s]\n", g_sedp_string_buf);
#endif
      }
      else
        FREERTPS_ERROR("couldn't parse type name of length %d\n", s->len);
    }
    else if (pid == FRUDP_PID_RELIABILITY)
    {
      frudp_qos_reliability_t *qos = (frudp_qos_reliability_t *)pval;
      if (qos->kind == FRUDP_QOS_RELIABILITY_KIND_BEST_EFFORT)
      {
#ifdef SEDP_VERBOSE
        printf("    reliability QoS: [best-effort]\n");
#endif
      }
      else if (qos->kind == FRUDP_QOS_RELIABILITY_KIND_RELIABLE)
      {
#ifdef SEDP_VERBOSE
        printf("    reliability QoS: [reliable]\n");
#endif
      }
      else
        FREERTPS_ERROR("unhandled reliability kind: %d\n", qos->kind);
    }
    else if (pid == FRUDP_PID_HISTORY)
    {
      frudp_qos_history_t *qos = (frudp_qos_history_t *)pval;
      if (qos->kind == FRUDP_QOS_HISTORY_KIND_KEEP_LAST)
      {
#ifdef SEDP_VERBOSE
        printf("    history QoS: [keep last %d]\n", qos->depth);
#endif
      }
      else if (qos->kind == FRUDP_QOS_HISTORY_KIND_KEEP_ALL)
      {
#ifdef SEDP_VERBOSE
        printf("    history QoS: [keep all]\n");
#endif
      }
      else
        FREERTPS_ERROR("unhandled history kind: %d\n", qos->kind);
    }
    else if (pid == FRUDP_PID_TRANSPORT_PRIORITY)
    {
#ifdef SEDP_VERBOSE
      uint32_t priority = *((uint32_t *)pval);
      printf("    transport priority: %d\n", priority);
#endif
    }

    // now, advance to next item in list...
    item = (frudp_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
}

static void frudp_sedp_bcast()
{
  //frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_frudp_discovery_tx_buf);
  //fr_time_t t = fr_time_now();
}

void sedp_publish_subscription(frudp_userland_subscription_request_t *sub_req)
{
  frudp_submsg_data_t *d = (frudp_submsg_data_t *)g_sedp_sub_buf;
  d->header.id = FRUDP_SUBMSG_ID_DATA;
  d->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN |
                    //FRUDP_FLAGS_INLINE_QOS    |
                    FRUDP_FLAGS_DATA_PRESENT  ;
  d->header.len = 0;
  d->extraflags = 0;
  d->octets_to_inline_qos = 16; // ?
  d->reader_id = g_sedp_sub_reader_id;
  d->writer_id = g_sedp_sub_writer_id;
  d->writer_sn = g_frudp_sequence_number_unknown;
  //frudp_parameter_list_item_t *inline_qos_param =
  //  (frudp_parameter_list_item_t *)d->data;
  /*
  inline_qos_param->pid = FRUDP_PID_KEY_HASH;
  inline_qos_param->len = 16;
  memcpy(inline_qos_param->value, &reader_guid, 16);
  */
  /////////////////////////////////////////////////////////////
  frudp_encapsulation_scheme_t *scheme =
    (frudp_encapsulation_scheme_t *)((uint8_t *)d->data);
  scheme->scheme = htons(FRUDP_ENCAPSULATION_SCHEME_PL_CDR_LE);
  scheme->options = 0;
  /////////////////////////////////////////////////////////////
  frudp_parameter_list_item_t *param =
    (frudp_parameter_list_item_t *)(((uint8_t *)scheme) + sizeof(*scheme));
  /////////////////////////////////////////////////////////////
  param->pid = FRUDP_PID_PROTOCOL_VERSION;
  param->len = 4;
  param->value[0] = 2;
  param->value[1] = 1;
  param->value[2] = param->value[3] = 0; // pad to 4-byte boundary
  /////////////////////////////////////////////////////////////
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_VENDOR_ID;
  param->len = 4;
  param->value[0] = (FREERTPS_VENDOR_ID >> 8) & 0xff;
  param->value[1] = FREERTPS_VENDOR_ID & 0xff;
  param->value[2] = param->value[3] = 0; // pad to 4-byte boundary
  /////////////////////////////////////////////////////////////
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_ENDPOINT_GUID;
  param->len = 16;
  frudp_guid_t reader_guid;
  reader_guid.guid_prefix = g_frudp_config.guid_prefix;
  reader_guid.entity_id = sub_req->entity_id;
  memcpy(param->value, &reader_guid, 16);
  /////////////////////////////////////////////////////////////
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_TOPIC_NAME;
  int topic_len = strlen(sub_req->topic_name);
  *((uint32_t *)param->value) = topic_len + 1;
  memcpy(param->value + 4, sub_req->topic_name, topic_len + 1);
  param->len = (4 + topic_len + 3) & ~0x3; // params must be 32-bit aligned
  /////////////////////////////////////////////////////////////
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_TYPE_NAME;
  int type_len = strlen(sub_req->type_name);
  *((uint32_t *)param->value) = type_len + 1;
  memcpy(param->value + 4, sub_req->type_name, type_len + 1);
  param->len = (4 + type_len + 3) & ~0x3; // params must be 32-bit aligned
  /////////////////////////////////////////////////////////////
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_RELIABILITY;
  param->len = 12;
  frudp_qos_reliability_t *reliability = (frudp_qos_reliability_t *)param->value;
  reliability->kind = FRUDP_QOS_RELIABILITY_KIND_BEST_EFFORT;
  reliability->max_blocking_time.sec = 0;
  reliability->max_blocking_time.nanosec = 0x19999999; // todo: figure this out
  /////////////////////////////////////////////////////////////
  /*
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_PRESENTATION;
  param->len = 8;
  frudp_qos_presentation_t *presentation = (frudp_qos_presentation_t *)param->value;
  presentation->scope = FRUDP_QOS_PRESENTATION_SCOPE_TOPIC;
  presentation->coherent_access = 0;
  presentation->ordered_access = 0;
  */
  /////////////////////////////////////////////////////////////
  FRUDP_PLIST_ADVANCE(param);
  param->pid = FRUDP_PID_SENTINEL;
  param->len = 0;
  FRUDP_PLIST_ADVANCE(param);
  d->header.len = param->value - 4 - (uint8_t *)&d->extraflags;
  frudp_publish(g_sedp_subscription_pub, d);
}

#if 0
/////////////////////////////////////////////////////////////
PLIST_ADVANCE(param_list);
param_list->pid = FRUDP_PID_BUILTIN_ENDPOINT_SET;
param_list->len = 4;
uint32_t endpoint_set = 0x3f;
memcpy(param_list->value, &endpoint_set, 4);
/////////////////////////////////////////////////////////////
PLIST_ADVANCE(param_list);
param_list->pid = FRUDP_PID_SENTINEL;
param_list->len = 0;
//data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
PLIST_ADVANCE(param_list);
data_submsg->header.len = param_list->value - 4 - data_submsg->contents;
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

#endif
