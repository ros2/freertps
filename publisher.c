#include "freertps/publisher.h"
#include "freertps/config.h"
#include "freertps/id.h"
#include "freertps/participant.h"
#include <string.h>

frudp_publisher_t g_frudp_pubs[FRUDP_MAX_PUBLISHERS];
int g_frudp_num_pubs = 0;
static uint8_t g_publisher_tx_buf[2048] = {0}; // just for now... so bad...

frudp_publisher_t *frudp_create_publisher(const char *topic_name,
                                          const char *type_name,
                                          const frudp_entity_id_t writer_id,
                                          frudp_submsg_data_t **data_submsgs,
                                          const uint32_t num_data_submsgs)
{
  //printf("create publisher 0x%08x\n", htonl(writer_id.u));
  if (g_frudp_num_pubs >= FRUDP_MAX_PUBLISHERS)
    return NULL; // no room. sorry
  frudp_publisher_t *p = &g_frudp_pubs[g_frudp_num_pubs];
  p->topic_name = topic_name;
  p->type_name = type_name;
  p->data_submsgs = data_submsgs;
  p->num_data_submsgs = num_data_submsgs;
  if (writer_id.u == g_frudp_entity_id_unknown.u)
  {
    p->writer_id.s.kind = FRUDP_ENTITY_KIND_USER_WRITER_NO_KEY;
    p->writer_id.s.key[0] = g_frudp_next_user_entity_id++;
    p->writer_id.s.key[1] = 0; // todo: >8 bit ID's. consolidate this in id.c
    p->writer_id.s.key[2] = 0;
  }
  else
    p->writer_id.u = writer_id.u;
  p->next_submsg_idx = 0;
  p->next_sn.low = 1;
  p->next_sn.high = 0;
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
  if (submsg->writer_sn.low == g_frudp_sequence_number_unknown.low) // todo: 64
  {
    pub_submsg->writer_sn = pub->next_sn;
    pub->next_sn.low++; // todo: > 32 bits
  }
  memcpy(pub_submsg->data, submsg->data, submsg->header.len - sizeof(frudp_submsg_data_t) + 4);
  //pub_sample->data_len = sample->data_len;
  // TODO: now, send DATA and HEARTBEAT submessages
  printf("frudp publish %d bytes, seq num %d\n",
         submsg->header.len,
         pub_submsg->writer_sn.low);
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
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_publisher_tx_buf);
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

frudp_publisher_t *frudp_publisher_from_writer_id(const frudp_entity_id_t id)
{
  //printf("pub from writer id 0x%08x\n", htonl(id.u));
  for (int i = 0; i < g_frudp_num_pubs; i++)
  {
    frudp_publisher_t *p = &g_frudp_pubs[i];
    //printf("  comp %d: 0x%08x ?= 0x%08x\n",
    //       i, htonl(id.u), htonl(p->writer_id.u));
    if (id.u == p->writer_id.u)
      return p;
  }
  return NULL;
}

void frudp_publisher_rx_acknack(frudp_publisher_t *pub,
                                frudp_submsg_acknack_t *acknack,
                                frudp_guid_prefix_t *guid_prefix)
{
  // see if we have any of the requested messages
  for (int req_seq_num = acknack->reader_sn_state.bitmap_base.low;
       req_seq_num < acknack->reader_sn_state.bitmap_base.low +
                     acknack->reader_sn_state.num_bits + 1;
       req_seq_num++)
  {
    //printf("     request for seq num %d\n", req_seq_num);
    for (int msg_idx = 0; msg_idx < pub->num_data_submsgs; msg_idx++)
    {
      frudp_submsg_data_t *data = pub->data_submsgs[msg_idx];
      //printf("       %d ?= %d\n", req_seq_num, data->writer_sn.low);
      if (data->writer_sn.low == req_seq_num)
      {
        //printf("        found it in the history cache! now i need to tx\n");

        //printf("about to look up guid prefix: ");
        //print_guid_prefix(guid_prefix);
        //printf("\n");

        // look up the GUID prefix of this reader, so we can call them back
        frudp_participant_t *part = frudp_participant_find(guid_prefix);
        if (!part)
        {
          printf("      woah there partner. you from around these parts?\n");
          return;
        }

        frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_publisher_tx_buf);
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
        memcpy(&msg->submsgs[submsg_wpos], data, 4 + data->header.len);
        //data_submsg, submsg, 4 + submsg->header.len);
        submsg_wpos += 4 + data->header.len;

        frudp_submsg_heartbeat_t *hb_submsg = (frudp_submsg_heartbeat_t *)&msg->submsgs[submsg_wpos];
        hb_submsg->header.id = FRUDP_SUBMSG_ID_HEARTBEAT;
        hb_submsg->header.flags = 0x3; // todo: spell this out
        hb_submsg->header.len = 28;
        hb_submsg->reader_id = data->reader_id;
        hb_submsg->writer_id = data->writer_id;
        hb_submsg->first_sn.low = 1; // todo
        hb_submsg->first_sn.high = 0; // todo
        hb_submsg->last_sn = data->writer_sn;
        static int hb_count = 0;
        hb_submsg->count = hb_count++;

        submsg_wpos += 4 + hb_submsg->header.len;

        int payload_len = &msg->submsgs[submsg_wpos] - ((uint8_t *)msg);
        //printf("         sending %d bytes\n", payload_len);
        frudp_tx(part->metatraffic_unicast_locator.addr.udp4.addr,
                 part->metatraffic_unicast_locator.port,
                 (const uint8_t *)msg, payload_len);
      }
    }
  }
}
