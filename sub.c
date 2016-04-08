#include "freertps/sub.h"
#include "freertps/udp.h"
#include "freertps/id.h"
#include "freertps/sedp.h"
#include "freertps/bswap.h"

frudp_sub_t g_frudp_subs[FRUDP_MAX_SUBS];
uint32_t g_frudp_num_subs = 0;

frudp_reader_t g_frudp_readers[FRUDP_MAX_READERS];
uint32_t g_frudp_num_readers = 0;

///////////////////////////////////////////////////////////////////////////

void frudp_add_reader(const frudp_reader_t *match)
{
  if (g_frudp_num_readers >= FRUDP_MAX_READERS)
    return;
  // make sure that in the meantime, we haven't already added this
  bool found = false;
  for (unsigned j = 0; !found && j < g_frudp_num_readers; j++)
  {
    frudp_reader_t *r = &g_frudp_readers[j];
    if (frudp_guid_identical(&r->writer_guid, &match->writer_guid))
      found = true;
  }
  if (found)
  {
    printf("found reader already; skipping duplicate add\n");
    return;
  }

  g_frudp_readers[g_frudp_num_readers] = *match;
  g_frudp_num_readers++;
  /*
  printf("add_reader(");
  frudp_print_guid(&match->writer_guid);
  printf(" => %08x)\r\n", (unsigned)freertps_htonl(match->reader_eid.u));
  */
}

void frudp_add_user_sub(const char *topic_name,
                        const char *type_name,
                        freertps_msg_cb_t msg_cb)
{
  frudp_eid_t sub_eid = frudp_create_user_id
                          (FRUDP_ENTITY_KIND_USER_READER_NO_KEY);
  printf("frudp_add_user_sub(%s, %s) on EID 0x%08x\r\n",
      topic_name, type_name, (unsigned)freertps_htonl(sub_eid.u));
  frudp_sub_t sub;
  // for now, just copy the pointers. maybe in the future we can/should have
  // an option for storage of various kind (static, malloc, etc.) for copies.
  sub.topic_name = topic_name;
  sub.type_name = type_name;
  sub.reader_eid = sub_eid;
  sub.msg_cb = msg_cb;
  sub.data_cb = NULL;
  sub.reliable = false;
  frudp_add_sub(&sub);
  //sedp_publish_sub(&sub); // can't do this yet; spdp hasn't started bcast
}

void frudp_add_sub(const frudp_sub_t *s)
{
  if (g_frudp_num_subs >= FRUDP_MAX_SUBS - 1)
    return; // no room. sorry.
  g_frudp_subs[g_frudp_num_subs] = *s;
  printf("sub %d: reader_eid = 0x%08x\r\n",
      g_frudp_num_subs, freertps_htonl((unsigned)s->reader_eid.u));
  g_frudp_num_subs++;
  //frudp_subscribe(s->entity_id, g_frudp_entity_id_unknown, NULL, s->msg_cb);
}

void frudp_print_readers(void)
{
  for (unsigned i = 0; i < g_frudp_num_readers; i++)
  {
    frudp_reader_t *match = &g_frudp_readers[i];
    printf("    sub %d: writer = ", (int)i); //%08x, reader = %08x\n",
    frudp_print_guid(&match->writer_guid);
    printf(" => %08x\r\n", (unsigned)freertps_htonl(match->reader_eid.u));
  }
}

///////////////////////////////////////////////////////////////////////////
/*
static void frudp_add_userland_subscription(
                                frudp_userland_subscription_request_t *s);
frudp_userland_subscription_request_t g_frudp_userland_subs[FRUDP_MAX_USERLAND_SUBS];
uint32_t g_frudp_num_userland_subs = 0;
*/
///////////////////////////////////////////////////////////////////////////

/*
NEED TO START RE-STRUCTURING STUFF HERE. THE SUBSCRIPTION OBJECTS SHOULD
HOLD ONLY THE "DESIRE" FOR A SUBSCRIPTION, AND FEED INTO A MATCHED-READER
OBJECT AT SOME POINT WHEN WE ACTUALLY HEAR ABOUT OTHER ENDPOINTS.
*/

/*
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
*/
