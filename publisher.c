#include "freertps/publisher.h"
#include "freertps/config.h"
#include "freertps/id.h"
#include <string.h>

frudp_publisher_t g_frudp_pubs[FRUDP_MAX_PUBLISHERS];
int g_frudp_num_pubs = 0;

frudp_publisher_t *frudp_create_publisher(const char *topic_name,
                                          const char *type_name,
                                          const frudp_entity_id_t writer_id,
                                          frudp_submsg_data_t **data_submsgs,
                                          const uint32_t num_data_submsgs)
{
  if (g_frudp_num_pubs >= FRUDP_MAX_PUBLISHERS)
    return NULL; // no room. sorry
  frudp_publisher_t *p = &g_frudp_pubs[g_frudp_num_pubs];
  p->topic_name = topic_name;
  p->type_name = type_name;
  p->data_submsgs = data_submsgs;
  p->writer_id.s.kind = FRUDP_ENTITY_KIND_USER_WRITER_NO_KEY;
  p->writer_id.s.key[0] = g_frudp_next_user_entity_id++;
  p->writer_id.s.key[1] = 0; // todo: >8 bit ID's. consolidate this in id.c
  p->writer_id.s.key[2] = 0;
  p->next_submsg_idx = 0;
  g_frudp_num_pubs++;
  return p;
}

void frudp_publish(frudp_publisher_t *pub, frudp_submsg_data_t *submsg)
{
  // (todo: allow people to stuff the message directly in the publisher and
  // call this function with sample set to NULL to indicate this)

  // find first place we can buffer this sample
  pub->max_tx_sn_avail.low++; // TODO: care about sample counts over 32 bits
  frudp_submsg_data_t *pub_submsg = pub->data_submsgs[pub->next_submsg_idx];
  *pub_submsg = *submsg;
  memcpy(pub_submsg->data, submsg->data, submsg->header.len - sizeof(frudp_submsg_data_t));
  //pub_sample->data_len = sample->data_len;
  // TODO: now, send DATA and HEARTBEAT submessages
  printf("frudp publish %d bytes\n", submsg->header.len);
/*
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  //data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
  //printf("len = %d\n", data_submsg->header.len);
  /////////////////////////////////////////////////////////////
  //int payload_len = ((uint8_t *)param_list) - ((uint8_t *)msg->submsgs);
  //int payload_len = ((uint8_t *)next_submsg_ptr) - ((uint8_t *)msg->submsgs);
  int payload_len = ((uint8_t *)next_submsg_ptr) - ((uint8_t *)msg);
  frudp_tx(inet_addr("239.255.0.1"), frudp_mcast_builtin_port(),
           (const uint8_t *)msg, payload_len);
*/
  static uint8_t fixme_buf[1500] = {0}; // just for now...
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)fixme_buf);
  fr_time_t t = fr_time_now();
  uint16_t submsg_wpos = 0;

  frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
  ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
  ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  ts_submsg->header.len = 8;
  memcpy(ts_submsg->contents, &t, 8);
  submsg_wpos += 4 + 8;
  ///////////////////////////////////////////////////////////////////////
  //frudp_submsg_data_t *data_submsg = (frudp_submsg_data_t *)&msg->submsgs[submsg_wpos];
  memcpy(&msg->submsgs[submsg_wpos], submsg, 4 + submsg->header.len);
  //data_submsg, submsg, 4 + submsg->header.len);
  submsg_wpos += 4 + submsg->header.len;

  frudp_submsg_heartbeat_t *hb_submsg = (frudp_submsg_heartbeat_t *)&msg->submsgs[submsg_wpos];
  hb_submsg->header.id = FRUDP_SUBMSG_ID_HEARTBEAT;
  hb_submsg->header.flags = 0x3; // todo: spell this out
  hb_submsg->header.len = 28;
  hb_submsg->reader_id = submsg->reader_id;
  hb_submsg->writer_id = submsg->writer_id;
  hb_submsg->first_sn.low = 1; // todo
  hb_submsg->first_sn.high = 0; // todo
  hb_submsg->last_sn = submsg->writer_sn;
  static int hb_count = 0;
  hb_submsg->count = hb_count++;

  submsg_wpos += 4 + hb_submsg->header.len;

  int payload_len = &msg->submsgs[submsg_wpos] - ((uint8_t *)msg);
  frudp_tx(htonl(FRUDP_DEFAULT_MCAST_GROUP), frudp_mcast_builtin_port(),
           (const uint8_t *)msg, payload_len);

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


}
