#include "freertps/cdr_string.h"
#include "freertps/freertps.h"
#include "freertps/guid.h"
#include "freertps/iterator.h"
#include "freertps/participant_proxy.h"
#include "freertps/qos.h"
#include "freertps/reader_proxy.h"
#include "freertps/sedp.h"
#include "freertps/spdp.h"
#include "freertps/writer_proxy.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////
// local constants
/*
static const fr_entity_id_t g_fr_sedp_pub_writer_id = { .u = 0xc2030000 };
static const fr_entity_id_t g_fr_sedp_pub_reader_id = { .u = 0xc7030000 };
static const fr_entity_id_t g_fr_sedp_sub_writer_id = { .u = 0xc2040000 };
static const fr_entity_id_t g_fr_sedp_sub_reader_id = { .u = 0xc7040000 };
*/
#define FR_MAX_TOPIC_NAME_LEN 128
#define FR_MAX_TYPE_NAME_LEN  128

////////////////////////////////////////////////////////////////////////////
// local functions
static void fr_sedp_rx_pub_data(struct fr_receiver *rcvr,
                                const struct fr_submessage *submsg,
                                const uint16_t scheme,
                                const uint8_t *data);
static void fr_sedp_rx_sub_data(struct fr_receiver *rcvr,
                                const struct fr_submessage *submsg,
                                const uint16_t scheme,
                                const uint8_t *data);
static void fr_sedp_rx_pubsub_data(struct fr_receiver *rcvr,
                                   const struct fr_submessage *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data,
                                   const bool is_pub);
////////////////////////////////////////////////////////////////////////////

void fr_sedp_init()
{
  FREERTPS_INFO("fr_sedp_init()\r\n");

  struct fr_writer *pub_writer = fr_writer_create(
      NULL, NULL, FR_WRITER_TYPE_RELIABLE);
  pub_writer->endpoint.entity_id.u = FR_EID_WRITER_WRITER;
  fr_participant_add_writer(pub_writer);

  struct fr_writer *sub_writer = fr_writer_create(
      NULL, NULL, FR_WRITER_TYPE_RELIABLE);
  sub_writer->endpoint.entity_id.u = FR_EID_READER_WRITER;
  fr_participant_add_writer(sub_writer);

  struct fr_reader *pub_reader = fr_reader_create(
      NULL, NULL, FR_READER_TYPE_RELIABLE);
  pub_reader->endpoint.entity_id.u = FR_EID_WRITER_READER;
  pub_reader->data_rx_cb = fr_sedp_rx_pub_data;
  fr_participant_add_reader(pub_reader);

  struct fr_reader *sub_reader = fr_reader_create(
      NULL, NULL, FR_READER_TYPE_RELIABLE);
  sub_reader->endpoint.entity_id.u = FR_EID_READER_READER;
  sub_reader->data_rx_cb = fr_sedp_rx_sub_data;
  fr_participant_add_reader(sub_reader);
}

void fr_sedp_fini()
{
  FREERTPS_INFO("sedp fini\r\n");
}

//#define SEDP_PRINT_TOPICS
static void fr_sedp_rx_pub_data(struct fr_receiver *rcvr,
                                const struct fr_submessage *submsg,
                                const uint16_t scheme,
                                const uint8_t *data)
{
  fr_sedp_rx_pubsub_data(rcvr, submsg, scheme, data, true);
}

static void fr_sedp_rx_sub_data(struct fr_receiver *rcvr,
                                const struct fr_submessage *submsg,
                                const uint16_t scheme,
                                const uint8_t *data)
{
  fr_sedp_rx_pubsub_data(rcvr, submsg, scheme, data, false);
}

typedef struct {
  fr_guid_t guid;
  char topic_name[FR_MAX_TOPIC_NAME_LEN];
  char type_name[FR_MAX_TYPE_NAME_LEN];
} sedp_topic_info_t;

static sedp_topic_info_t g_topic_info;

static void fr_sedp_rx_pub_info(const sedp_topic_info_t *info)
{
  printf("sedp pub: [%s / %s]\n",
      info->topic_name ? info->topic_name : "",
      info->type_name ? info->type_name : "");
  for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.readers);
       it.data; fr_iterator_next(&it))
  {
    struct fr_reader *reader = it.data;
    /*
    printf("comparing incoming SEDP pub to our subscriber %s of type %s\r\n",
           (reader->topic_name ? reader->topic_name : "(no name)"),
           (reader->type_name  ? reader->type_name  : "(no type)"));
    */
    if (!reader->topic_name || !reader->type_name)
      continue; // sanity check. some built-ins don't have names.

    // look to see if we are subscribed to this topic
    if (strcmp(reader->topic_name, info->topic_name) ||
        strcmp(reader->type_name, info->type_name))
      continue; // didn't match topic name or didn't match type name

    printf("    hooray! found a topic we care about: [%s]\n",
        reader->topic_name);

    // see if we already have a matched reader for this writer
    bool found = false;
    for (struct fr_iterator mw_it = fr_iterator_begin(reader->matched_writers);
        mw_it.data; fr_iterator_next(&mw_it))
    {
      struct fr_writer_proxy *writer_proxy = mw_it.data;
      if (fr_guid_identical(&writer_proxy->remote_writer_guid, &info->guid))
      {
        printf("    already have this writer proxy!\n");
        found = true;
      }
    }
    if (found)
      continue;
    printf("adding this remote writer ");
    fr_guid_print(&info->guid);
    printf(" to our reader\n");
    struct fr_writer_proxy wp;
    fr_guid_copy(&info->guid, &wp.remote_writer_guid);
    wp.highest_sequence_number = 0;
    fr_container_append(reader->matched_writers,
        &wp, sizeof(struct fr_writer_proxy), FR_CFLAGS_NONE);
  }

#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
      if (!found)
      {
        fr_reader_t r;
        r.writer_guid = info->guid;
        r.reader_entity_id = sub->reader_entity_id;
        r.max_rx_sn.high = 0;
        r.max_rx_sn.low = 0;
        r.data_cb = sub->data_cb;
        r.msg_cb = sub->msg_cb;
        r.reliable = sub->reliable;
        fr_add_reader(&r);
      }
      else
        printf("boring, we already knew about it.\n");
    }
  }
#endif
}

//#define SEDP_VERBOSE
static void fr_sedp_rx_sub_info(const sedp_topic_info_t *info)
{
  printf("sedp sub: [%s / %s]\r\n",
      info->topic_name ? info->topic_name : "",
      info->type_name  ? info->type_name  : "");
  // look to see if we publish this topic
  for (struct fr_iterator it = fr_iterator_begin(g_fr_participant.writers);
       it.data; fr_iterator_next(&it))
  {
    struct fr_writer *writer = it.data;
#ifdef SEDP_VERBOSE
    printf("comparing incoming SEDP sub to our publisher %s of type %s\r\n",
           (writer->topic_name ? writer->topic_name : "(no name)"),
           (writer->type_name  ? writer->type_name  : "(no type)"));
#endif
    if (!writer->topic_name || !writer->type_name)
      continue; // sanity check. some built-ins don't have names.
    if (strcmp(writer->topic_name, info->topic_name))
      continue; // not the same topic. move along.
    if (strcmp(writer->type_name, info->type_name))
    {
      printf("    SEDP type mismatch: [%s] != [%s]\r\n",
             writer->type_name, info->type_name);
      continue;
    }
    printf("    hooray! heard a request for a topic we publish: [%s]\r\n",
           writer->topic_name);
    // see if we already have a matched-reader for this subscriber
    // TODO: for now, this only handles unreliable readers !
    // first, try to find this participant in our matched_participant list
    struct fr_participant_proxy *matched_participant = NULL;
    for (struct fr_iterator pp_it =
         fr_iterator_begin(g_fr_participant.matched_participants);
         !matched_participant && pp_it.data; fr_iterator_next(&pp_it))
    {
      struct fr_participant_proxy *pp = pp_it.data;
      if (fr_guid_prefix_identical(&pp->guid_prefix, &info->guid.prefix))
        matched_participant = pp;
    }
    if (!matched_participant)
    {
      printf("    couldn't find SPDP participant ");
      fr_guid_print_prefix(&info->guid.prefix);
      return; // adios amigo
    }
      
    bool already_sending = false;
    // make sure we aren't already sending to this participant
    for (struct fr_iterator rl_it = fr_iterator_begin(writer->reader_locators);
         !already_sending && rl_it.data; fr_iterator_next(&rl_it))
    {
      struct fr_reader_locator *reader_locator = rl_it.data;
      // compare against all known locators of matched_participant
      if (fr_locator_identical(&reader_locator->locator,
            &matched_participant->metatraffic_unicast_locator) || 
          fr_locator_identical(&reader_locator->locator,
            &matched_participant->metatraffic_multicast_locator) ||
          fr_locator_identical(&reader_locator->locator,
            &matched_participant->default_unicast_locator) ||
          fr_locator_identical(&reader_locator->locator,
            &matched_participant->default_multicast_locator))
        already_sending = true;
    }
    if (already_sending)
    {
      printf("  already had it in our list of readers\n");
      return;
    }
    // create a reader locator and append it to this writer's locator list
    struct fr_reader_locator reader_locator;
    fr_reader_locator_init(&reader_locator);
    struct fr_locator *participant_locator = NULL;
    if (!info->topic_name || !info->type_name) // built-in topic
      participant_locator = &matched_participant->metatraffic_unicast_locator;
    else
      participant_locator = &matched_participant->default_unicast_locator;
    fr_locator_set_udp4(&reader_locator.locator,
        participant_locator->addr.udp4.addr,
        participant_locator->port);
    fr_writer_add_reader_locator(writer, &reader_locator);
  }
}

static void fr_sedp_rx_pubsub_data(struct fr_receiver *rcvr,
                                   const struct fr_submessage *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data,
                                   const bool is_pub)
{
#ifdef SEDP_VERBOSE
  printf("  sedp_writer data rx\r\n");
#endif
  if (scheme != FR_SCHEME_PL_CDR_LE)
  {
    FREERTPS_ERROR("expected sedp data to be PL_CDR_LE, found 0x%04x\n",
        (unsigned)scheme);
    return;
  }
  memset(&g_topic_info, 0, sizeof(sedp_topic_info_t));
  struct fr_parameter_list_item *item = (struct fr_parameter_list_item *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    const fr_parameter_id_t pid = item->pid;
    if (pid == FR_PID_SENTINEL)
      break;
    const uint8_t *pval = item->value;
    if (pid == FR_PID_ENDPOINT_GUID)
    {
      struct fr_guid *guid = (struct fr_guid *)pval;
      memcpy(&g_topic_info.guid, guid, sizeof(fr_guid_t));
//#ifdef SEDP_VERBOSE
      //memcpy(&part->guid_prefix, &guid->guid_prefix, FR_GUID_PREFIX_LEN);
      printf("    endpoint guid ");
      fr_guid_print(guid);
      printf("\n");
//#endif
      //if (guid->entity_id.u == 0x03010000)
      //  printf("found entity 0x103\n");
    }
    else if (pid == FR_PID_TOPIC_NAME)
    {
      if (fr_cdr_string_parse(g_topic_info.topic_name,
                              sizeof(g_topic_info.topic_name),
                              (struct fr_cdr_string *)pval))
      {
//#ifdef SEDP_PRINT_TOPICS
        printf("    topic name: [%s]\r\n", g_topic_info.topic_name);
//#endif
      }
      else
        FREERTPS_ERROR("couldn't parse topic name\n");
    }
    else if (pid == FR_PID_TYPE_NAME)
    {
      if (fr_cdr_string_parse(g_topic_info.type_name,
                              sizeof(g_topic_info.type_name),
                              (struct fr_cdr_string *)pval))
      {
//#ifdef SEDP_VERBOSE
        printf("    type name: [%s]\r\n", g_topic_info.type_name);
//#endif
      }
      else
        FREERTPS_ERROR("couldn't parse type name\r\n");
    }
    else if (pid == FR_PID_RELIABILITY)
    {
      struct fr_qos_reliability *qos = (struct fr_qos_reliability *)pval;
      if (qos->kind == FR_QOS_RELIABILITY_KIND_BEST_EFFORT)
      {
#ifdef SEDP_VERBOSE
        printf("    reliability QoS: [best-effort]\r\n");
#endif
      }
      else if (qos->kind == FR_QOS_RELIABILITY_KIND_RELIABLE)
      {
#ifdef SEDP_VERBOSE
        printf("    reliability QoS: [reliable]\r\n");
#endif
      }
      else
        FREERTPS_ERROR("unhandled reliability kind: %d\r\n", (int)qos->kind);
    }
    else if (pid == FR_PID_HISTORY)
    {
      fr_qos_history_t *qos = (fr_qos_history_t *)pval;
      if (qos->kind == FR_QOS_HISTORY_KIND_KEEP_LAST)
      {
#ifdef SEDP_VERBOSE
        printf("    history QoS: [keep last %d]\r\n", (int)qos->depth);
#endif
      }
      else if (qos->kind == FR_QOS_HISTORY_KIND_KEEP_ALL)
      {
#ifdef SEDP_VERBOSE
        printf("    history QoS: [keep all]\r\n");
#endif
      }
      else
        FREERTPS_ERROR("unhandled history kind: %d\r\n", (int)qos->kind);
    }
    else if (pid == FR_PID_TRANSPORT_PRIORITY)
    {
#ifdef SEDP_VERBOSE
      uint32_t priority = *((uint32_t *)pval);
      printf("    transport priority: %d\r\n", (int)priority);
#endif
    }
    // now, advance to next item in list...
    item = (struct fr_parameter_list_item *)(((uint8_t *)item) + 4 + item->len);
  }

  // make sure we have received all necessary parameters
  if (!strlen(g_topic_info.type_name) ||
      !strlen(g_topic_info.topic_name) ||
      fr_guid_identical(&g_topic_info.guid, &g_fr_guid_unknown))
  {
    FREERTPS_ERROR("insufficient SEDP information\r\n");
    return;
  }
  if (is_pub) // this is information about someone else's publication
    fr_sedp_rx_pub_info(&g_topic_info);
  else // this is information about someone else's subscription
    fr_sedp_rx_sub_info(&g_topic_info);
}

#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
static void sedp_publish(const char *topic_name,
                         const char *type_name,
                         fr_pub_t *pub,
                         const fr_entity_id_t entity_id,
                         const bool is_pub) // is this for a pub or a sub
{
  // first make sure we have an spdp packet out first
  printf("sedp publish [%s] via SEDP EID 0x%08x\r\n",
      topic_name, (unsigned)freertps_htonl(pub->writer_entity_id.u));
  fr_submsg_data_t *d = (fr_submsg_data_t *)g_sedp_msg_buf;
  d->header.id = FR_SUBMSG_ID_DATA;
  d->header.flags = FR_FLAGS_LITTLE_ENDIAN |
                    //FR_FLAGS_INLINE_QOS    |
                    FR_FLAGS_DATA_PRESENT  ;
  d->header.len = 0;
  d->extraflags = 0;
  d->octets_to_inline_qos = 16; // ?
  d->reader_id = is_pub ? g_sedp_pub_reader_id : g_sedp_sub_reader_id;
  d->writer_id = is_pub ? g_sedp_pub_writer_id : g_sedp_sub_writer_id;
  //d->writer_sn = g_fr_sn_unknown;
  d->writer_sn.high = 0;
  d->writer_sn.low = 0; // todo: increment this
  //fr_parameter_list_item_t *inline_qos_param =
  //  (fr_parameter_list_item_t *)d->data;
  /*
  inline_qos_param->pid = FR_PID_KEY_HASH;
  inline_qos_param->len = 16;
  memcpy(inline_qos_param->value, &reader_guid, 16);
  */
  /////////////////////////////////////////////////////////////
  fr_encapsulation_scheme_t *scheme =
    (fr_encapsulation_scheme_t *)((uint8_t *)d->data);
  scheme->scheme = freertps_htons(FR_SCHEME_PL_CDR_LE);
  scheme->options = 0;
  /////////////////////////////////////////////////////////////
  fr_parameter_list_item_t *param =
    (fr_parameter_list_item_t *)(((uint8_t *)scheme) + sizeof(*scheme));
  /////////////////////////////////////////////////////////////
  param->pid = FR_PID_PROTOCOL_VERSION;
  param->len = 4;
  param->value[0] = 2;
  param->value[1] = 1;
  param->value[2] = param->value[3] = 0; // pad to 4-byte boundary
  /////////////////////////////////////////////////////////////
  FR_PLIST_ADVANCE(param);
  param->pid = FR_PID_VENDOR_ID;
  param->len = 4;
  param->value[0] = (FREERTPS_VENDOR_ID >> 8) & 0xff;
  param->value[1] = FREERTPS_VENDOR_ID & 0xff;
  param->value[2] = param->value[3] = 0; // pad to 4-byte boundary
  /////////////////////////////////////////////////////////////
  FR_PLIST_ADVANCE(param);
  param->pid = FR_PID_ENDPOINT_GUID;
  param->len = 16;
  fr_guid_t guid;
  guid.prefix = g_fr_config.guid_prefix;
  guid.entity_id = entity_id;
  memcpy(param->value, &guid, 16);
  //printf("reader_guid = 0x%08x\n", htonl(reader_guid.entity_id.u));
  /////////////////////////////////////////////////////////////
  if (topic_name)
  {
    FR_PLIST_ADVANCE(param);
    param->pid = FR_PID_TOPIC_NAME;
    int topic_len = topic_name ? strlen(topic_name) : 0;
    uint32_t *param_topic_len = (uint32_t *)param->value;
    *param_topic_len = topic_len + 1;
    //*((uint32_t *)param->value) = topic_len + 1;
    memcpy(param->value + 4, topic_name, topic_len + 1);
    //param->value[4 + topic_len + 1] = 0; // null-terminate plz
    param->len = (4 + topic_len + 1 + 3) & ~0x3; // params must be 32-bit aligned
  }
  /////////////////////////////////////////////////////////////
  if (type_name)
  {
    FR_PLIST_ADVANCE(param);
    param->pid = FR_PID_TYPE_NAME;
    int type_len = strlen(type_name);
    uint32_t *value = (uint32_t *)param->value;
    *value = type_len + 1;
    memcpy(param->value + 4, type_name, type_len + 1);
    param->len = (4 + type_len + 1 + 3) & ~0x3; // params must be 32-bit aligned
  }
  /////////////////////////////////////////////////////////////
  // todo: follow the "reliable" flag in the subscription structure
  FR_PLIST_ADVANCE(param);
  param->pid = FR_PID_RELIABILITY;
  param->len = 12;
  fr_qos_reliability_t *reliability = (fr_qos_reliability_t *)param->value;
  reliability->kind = FR_QOS_RELIABILITY_KIND_BEST_EFFORT;
  reliability->max_blocking_time.seconds = 0;
  reliability->max_blocking_time.fraction = 0x19999999; // todo: figure this out
  /////////////////////////////////////////////////////////////
  /*
  FR_PLIST_ADVANCE(param);
  param->pid = FR_PID_PRESENTATION;
  param->len = 8;
  fr_qos_presentation_t *presentation = (fr_qos_presentation_t *)param->value;
  presentation->scope = FR_QOS_PRESENTATION_SCOPE_TOPIC;
  presentation->coherent_access = 0;
  presentation->ordered_access = 0;
  */
  /////////////////////////////////////////////////////////////
  FR_PLIST_ADVANCE(param);
  param->pid = FR_PID_SENTINEL;
  param->len = 0;
  FR_PLIST_ADVANCE(param);
  d->header.len = param->value - 4 - (uint8_t *)&d->extraflags;
  fr_publish(pub, d); // this will be either on the sub or pub publisher
}

void sedp_publish_sub(fr_sub_t *sub)
{
  if (!g_sedp_sub_pub)
  {
    printf("woah there partner.\r\n"
           "you need to call fr_part_create()\r\n");
    return;
  }
  printf("sedp_publish_sub(%s)\r\n", sub->topic_name);
  sedp_publish(sub->topic_name,
               sub->type_name,
               g_sedp_sub_pub,
               sub->reader_entity_id,
               false); // false means "this is for a subscription"
}

void sedp_publish_pub(fr_pub_t *pub)
{
  if (!g_sedp_pub_pub)
  {
    printf("woah there partner.\r\n"
           "you need to call fr_part_create()\r\n");
    return;
  }
  printf("sedp_publish_pub(%s)\r\n", pub->topic_name);
  sedp_publish(pub->topic_name,
               pub->type_name,
               g_sedp_pub_pub,
               pub->writer_entity_id,
               true); // true means "this is for a publication"
}
#endif

void fr_sedp_add_builtin_endpoints(struct fr_guid_prefix *prefix)
{
  printf("adding endpoints for ");
  fr_guid_print_prefix(prefix);
  printf("\r\n");

/*
  struct fr_reader *pub_reader = fr_reader_create(
      NULL, NULL, FR_READER_TYPE_RELIABLE);
  pub_reader->endpoint.entity_id = g_fr_sedp_pub_reader_id;
  pub_reader->data_rx_cb = fr_sedp_rx_pub_data;
  // stuff matched_writer object now since we know prefix, etc.
*/
  struct fr_writer_proxy pub_writer_proxy;
  fr_guid_stuff(&pub_writer_proxy.remote_writer_guid, prefix,
      FR_EID_WRITER_WRITER);
  pub_writer_proxy.highest_sequence_number = 0;
  const union fr_entity_id wr_eid = { .u = FR_EID_WRITER_READER };
  struct fr_reader *pub_r = fr_participant_get_reader(wr_eid);
  fr_container_append(pub_r->matched_writers,
      &pub_writer_proxy, sizeof(struct fr_writer_proxy), FR_CFLAGS_NONE);

  struct fr_writer_proxy sub_writer_proxy;
  fr_guid_stuff(&sub_writer_proxy.remote_writer_guid, prefix, 
      FR_EID_READER_WRITER);
  sub_writer_proxy.highest_sequence_number = 0;
  const union fr_entity_id rr_eid = { .u = FR_EID_READER_READER };
  struct fr_reader *sub_r = fr_participant_get_reader(rr_eid);
  fr_container_append(sub_r->matched_writers,
      &sub_writer_proxy, sizeof(struct fr_writer_proxy), FR_CFLAGS_NONE);

  struct fr_reader_proxy sub_reader_proxy;
  sub_reader_proxy.expects_inline_qos = false;
  sub_reader_proxy.highest_seq_num_sent = 0;
  sub_reader_proxy.lowest_requested_change = 0;
  fr_guid_stuff(&sub_reader_proxy.remote_reader_guid, prefix, 
      FR_EID_READER_READER);
  const union fr_entity_id rw_eid = { .u = FR_EID_READER_WRITER };
  struct fr_writer *sub_w = fr_participant_get_writer(rw_eid);
  fr_container_append(sub_w->matched_readers,
      &sub_reader_proxy, sizeof(struct fr_reader_proxy), FR_CFLAGS_NONE);

  struct fr_reader_proxy pub_reader_proxy;
  pub_reader_proxy.expects_inline_qos = false;
  pub_reader_proxy.highest_seq_num_sent = 0;
  pub_reader_proxy.lowest_requested_change = 0;
  fr_guid_stuff(&pub_reader_proxy.remote_reader_guid, prefix, 
      FR_EID_WRITER_READER);
  const union fr_entity_id ww_eid = { .u = FR_EID_WRITER_WRITER };
  struct fr_writer *pub_w = fr_participant_get_writer(ww_eid);
  fr_container_append(pub_w->matched_readers,
      &pub_reader_proxy, sizeof(struct fr_reader_proxy), FR_CFLAGS_NONE);

  // now, bring these new reader proxies up to speed
  // TODO: follow 8.4.2.3.4 and pre-emptively send ACKNACKs to the remote
  // writers to get them to send us their SEDP messages...
  // TODO: add flag for each reader_proxy to keep track of if this ACKNACK
  // has been sent

#if HORRIBLY_BROKEN_DURING_HISTORYCACHE_REWRITE
  fr_stuff_guid(&pub_reader.writer_guid,
                   &part->guid_prefix,
                   &g_sedp_pub_writer_id);
  pub_reader.data_cb = fr_sedp_rx_pub_data;
  fr_add_reader(&pub_reader);

  fr_reader_t sub_reader; // this reads the remote peer's subscriptions
  sub_reader.writer_guid = g_fr_guid_unknown;
  fr_stuff_guid(&sub_reader.writer_guid,
                   &part->guid_prefix,
                   &g_sedp_sub_writer_id);
  sub_reader.reader_entity_id = g_sedp_sub_reader_id;
  sub_reader.max_rx_sn.low = 0;
  sub_reader.max_rx_sn.high = 0;
  sub_reader.data_cb = fr_sedp_rx_sub_data;
  sub_reader.msg_cb = NULL;
  sub_reader.reliable = true;
  fr_add_reader(&sub_reader);

  // blast our SEDP data at this participant
  fr_send_sedp_msgs(part);
#endif
}


#if 0
/////////////////////////////////////////////////////////////
PLIST_ADVANCE(param_list);
param_list->pid = FR_PID_BUILTIN_ENDPOINT_SET;
param_list->len = 4;
uint32_t endpoint_set = 0x3f;
memcpy(param_list->value, &endpoint_set, 4);
/////////////////////////////////////////////////////////////
PLIST_ADVANCE(param_list);
param_list->pid = FR_PID_SENTINEL;
param_list->len = 0;
//data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
PLIST_ADVANCE(param_list);
data_submsg->header.len = param_list->value - 4 - data_submsg->contents;
fr_submsg_t *next_submsg_ptr = (fr_submsg_t *)param_list;
/////////////////////////////////////////////////////////////
/*
ts_submsg = (fr_submsg_t *)param_list;
ts_submsg->header.id = FR_SUBMSG_ID_INFO_TS;
ts_submsg->header.flags = FR_FLAGS_LITTLE_ENDIAN;
ts_submsg->header.len = 8;
memcpy(ts_submsg->contents, &t, 8);
uint8_t *next_submsg_ptr = ((uint8_t *)param_list) + 4 + 8;
*/

#endif
