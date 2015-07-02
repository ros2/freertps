#include "freertps/subscription.h"
#include "freertps/udp.h"
#include "freertps/id.h"
#include "freertps/sedp.h"

frudp_matched_reader_t g_frudp_matched_readers[FRUDP_MAX_MATCHED_READERS];
unsigned g_frudp_subs_used = 0;

///////////////////////////////////////////////////////////////////////////
static void frudp_add_userland_subscription(
                                frudp_userland_subscription_request_t *s);
frudp_userland_subscription_request_t g_frudp_userland_subs[FRUDP_MAX_USERLAND_SUBS];
uint32_t g_frudp_num_userland_subs = 0;
///////////////////////////////////////////////////////////////////////////

NEED TO START RE-STRUCTURING STUFF HERE. THE SUBSCRIPTION OBJECTS SHOULD
HOLD ONLY THE "DESIRE" FOR A SUBSCRIPTION, AND FEED INTO A MATCHED-READER
OBJECT AT SOME POINT WHEN WE ACTUALLY HEAR ABOUT OTHER ENDPOINTS.

bool frudp_subscribe(const frudp_entity_id_t reader_id,
                     const frudp_entity_id_t writer_id,
                     const frudp_rx_data_cb_t data_cb,
                     const freertps_msg_cb_t msg_cb)
{
  if (g_frudp_subs_used >= FRUDP_MAX_SUBSCRIPTIONS)
    return false;
  frudp_subscription_t *sub = &g_frudp_subs[g_frudp_subs_used];
  sub->reader_id = reader_id;
  sub->writer_id = writer_id;
  sub->data_cb = data_cb;
  sub->msg_cb = msg_cb;
  sub->max_rx_sn.low = sub->max_rx_sn.high = 0;
  g_frudp_subs_used++;
  return true;
}

void frudp_create_subscription(const char *topic_name,
                               const char *type_name,
                               freertps_msg_cb_t msg_cb)
{
  printf("frudp_create_subscription(%s, %s)\n",
         topic_name, type_name);
  frudp_entity_id_t sub_entity_id;
  sub_entity_id.s.kind = FRUDP_ENTITY_KIND_USER_READER_NO_KEY; // has key? dunno
  sub_entity_id.s.key[0] = 0;
  sub_entity_id.s.key[1] = 0; // todo: >8 bit ID's
  sub_entity_id.s.key[2] = g_frudp_next_user_entity_id++;
  frudp_userland_subscription_request_t req;
  // for now, just copy the pointers. maybe in the future we can/should have
  // an option for storage of various kind (static, malloc, etc.) for copies.
  req.topic_name = topic_name;
  req.type_name = type_name;
  req.entity_id = sub_entity_id;
  req.msg_cb = msg_cb;
  frudp_add_userland_subscription(&req);
  sedp_publish_subscription(&req);
}

void frudp_add_userland_subscription(frudp_userland_subscription_request_t *s)
{
  if (g_frudp_num_userland_subs >= FRUDP_MAX_USERLAND_SUBS - 1)
    return; // no room. sorry.
  g_frudp_userland_subs[g_frudp_num_userland_subs] = *s;
  g_frudp_num_userland_subs++;
  frudp_subscribe(s->entity_id, g_frudp_entity_id_unknown, NULL, s->msg_cb);
}
