#include <stdio.h>
#include "freertps/pub.h"
#include "freertps/config.h"
#include "freertps/id.h"
#include "freertps/part.h"
#include "freertps/bswap.h"
#include <string.h>
#include "freertps/freertps.h"
#include "freertps/sedp.h"
#include "freertps/spdp.h"

frudp_writer_t g_frudp_writers[FRUDP_MAX_WRITERS];
uint32_t g_frudp_num_writers;

frudp_pub_t g_frudp_pubs[FRUDP_MAX_PUBS];
uint32_t g_frudp_num_pubs = 0;
static uint8_t g_pub_tx_buf[2048];
static uint8_t g_pub_user_tx_buf[2048];

///////////////////////////////////////////////////////////////////////////

frudp_pub_t *frudp_create_pub(const char *topic_name,
                              const char *type_name,
                              const frudp_eid_t writer_id,
                              frudp_submsg_data_t **data_submsgs,
                              const uint32_t num_data_submsgs)
{
  //printf("create pub 0x%08x\n", htonl(writer_id.u));
  if (g_frudp_num_pubs >= FRUDP_MAX_PUBS)
  {
    FREERTPS_ERROR("woah there partner. don't have space for more pubs.\n");
    return NULL; // no room. sorry
  }
  frudp_pub_t *p = &g_frudp_pubs[g_frudp_num_pubs];
  p->topic_name = topic_name;
  p->type_name = type_name;
  if (data_submsgs) // it's for a reliable connection
  {
    p->data_submsgs = data_submsgs;
    p->num_data_submsgs = num_data_submsgs;
    p->reliable = true;
  }
  else
  {
    // unreliable; fire-and-forget from the caller's memory
    p->data_submsgs = NULL;
    p->num_data_submsgs = 0;
    p->reliable = false;
  }
  if (writer_id.u == g_frudp_eid_unknown.u)
  {
    FREERTPS_ERROR("AWAY WITH YOU! <point scepter>\n");
    return NULL;
  }
  else
    p->writer_eid.u = writer_id.u;
  p->next_submsg_idx = 0;
  p->next_sn.low = 1;
  p->next_sn.high = 0;
  g_frudp_num_pubs++;
  return p;
}

frudp_pub_t *frudp_create_user_pub(const char *topic_name, 
                                   const char *type_name)
{
  printf("create_user_pub(%s, %s)\r\n", topic_name, type_name);
  frudp_pub_t *pub = frudp_create_pub(topic_name,
                                      type_name,
                                      frudp_create_user_id
                                        (FRUDP_ENTITY_KIND_USER_WRITER_NO_KEY),
                                      NULL,
                                      0);
  //sedp_publish_pub(pub); // can't do this yet; disco hasn't started
  return pub;
}

static int sedp_hb_count = 0;
void frudp_send_sedp_hb(frudp_part_t *part, bool with_msgs)
{
  if (with_msgs)
  {
    frudp_spdp_bcast();
    // we have just found out about a new participant. blast our SPDP and SEDP messages
    // at it to help it join quickly
  }

  // first, send the publications
  if (g_sedp_pub_pub->next_submsg_idx)
  {
    frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_pub_tx_buf);
    fr_time_t t = fr_time_now();
    uint16_t submsg_wpos = 0;

    frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
    ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
    ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
    ts_submsg->header.len = 8;
    memcpy(ts_submsg->contents, &t, 8);
    submsg_wpos += 4 + 8;

    frudp_submsg_heartbeat_t *hb_submsg = 
      (frudp_submsg_heartbeat_t *)&msg->submsgs[submsg_wpos];
    hb_submsg->header.id = FRUDP_SUBMSG_ID_HEARTBEAT;
    hb_submsg->header.flags = 0x1; // todo: spell this out
    hb_submsg->header.len = 28;
    hb_submsg->reader_id = g_sedp_pub_pub->data_submsgs[0]->reader_id;
    hb_submsg->writer_id = g_sedp_pub_pub->data_submsgs[0]->writer_id;
    hb_submsg->first_sn.low = 1; // todo
    hb_submsg->first_sn.high = 0; // todo
    hb_submsg->last_sn.low = g_sedp_pub_pub->next_submsg_idx;
    hb_submsg->last_sn.high = 0; // todo
    hb_submsg->count = sedp_hb_count ++;
    submsg_wpos += 4 + hb_submsg->header.len;

    if (with_msgs)
    {
#ifdef SEDP_VERBOSE
      printf("sending %d SEDP publication catchup messages\n",
          g_sedp_pub_pub->next_submsg_idx);
#endif // SEDP_VERBOSE      
      for (int i = 0; i < g_sedp_pub_pub->next_submsg_idx; i++)
      {
        // todo: make sure we don't overflow a single ethernet frame
        // since that would be most non-triumphant
        frudp_submsg_data_t *pub_submsg = g_sedp_pub_pub->data_submsgs[i];
        memcpy(&msg->submsgs[submsg_wpos], pub_submsg, 
            4 + pub_submsg->header.len);
        submsg_wpos += 4 + pub_submsg->header.len;
#ifdef SEDP_VERBOSE
	printf("sending %d SEDP publication catchup messages\n",
	    g_sedp_pub_pub->next_submsg_idx);
	printf("catchup SEDP msg %d addressed to reader EID 0x%08x\r\n",
	    i, freertps_htonl((unsigned)pub_submsg->reader_id.u));
#endif // SEDP_VERBOSE
      }
    }
    int payload_len = &msg->submsgs[submsg_wpos] - ((uint8_t *)msg);
    uint32_t dst_addr = part->metatraffic_unicast_locator.addr.udp4.addr;
    uint16_t dst_port = part->metatraffic_unicast_locator.port;
#ifdef SEDP_VERBOSE
    printf("sending %d bytes of SEDP pub catchup messages or HB to 0x%08x:%d\r\n",
           payload_len, dst_addr, dst_port);
#endif // SEDP_VERBOSE
    frudp_tx(dst_addr, dst_port, (const uint8_t *)msg, payload_len);
  }
  else
  {
#ifdef SEDP_VERBOSE
    if (with_msgs)
    {
      printf("no SEDP pub data to send to new participant\r\n");
    }
#endif // SEDP_VERBOSE
  }

  // now, send the subscriptions
  if (g_sedp_sub_pub->next_submsg_idx)
  {
    frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_pub_tx_buf);
    fr_time_t t = fr_time_now();
    uint16_t submsg_wpos = 0;

    frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
    ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
    ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
    ts_submsg->header.len = 8;
    memcpy(ts_submsg->contents, &t, 8);
    submsg_wpos += 4 + 8;

    frudp_submsg_heartbeat_t *hb_submsg = 
      (frudp_submsg_heartbeat_t *)&msg->submsgs[submsg_wpos];
    hb_submsg->header.id = FRUDP_SUBMSG_ID_HEARTBEAT;
    hb_submsg->header.flags = 0x1; // todo: spell this out
    hb_submsg->header.len = 28;
    hb_submsg->reader_id = g_sedp_sub_pub->data_submsgs[0]->reader_id;
    hb_submsg->writer_id = g_sedp_sub_pub->data_submsgs[0]->writer_id;
    hb_submsg->first_sn.low = 1; // todo
    hb_submsg->first_sn.high = 0; // todo
    hb_submsg->last_sn.low = g_sedp_sub_pub->next_submsg_idx;
    hb_submsg->last_sn.high = 0; // todo
    hb_submsg->count = sedp_hb_count ++;
    submsg_wpos += 4 + hb_submsg->header.len;

    if (with_msgs)
    {
#ifdef SEDP_VERBOSE
     printf("sending %d SEDP subscription catchup messages\n",
	    g_sedp_sub_pub->next_submsg_idx);
#endif // SEDP_VERBOSE
      for (int i = 0; i < g_sedp_sub_pub->next_submsg_idx; i++)
      {
        // todo: make sure we don't overflow a single ethernet frame,
        // since that would be most non-triumphant
        frudp_submsg_data_t *sub_submsg = g_sedp_sub_pub->data_submsgs[i];
        memcpy(&msg->submsgs[submsg_wpos], sub_submsg, 
            4 + sub_submsg->header.len);
        submsg_wpos += 4 + sub_submsg->header.len;
#ifdef SEDP_VERBOSE
	printf("catchup SEDP msg %d addressed to reader EID 0x%08x\r\n",
	    i, freertps_htonl((unsigned)sub_submsg->reader_id.u));
#endif // SEDP_VERBOSE
      }
    }

    int payload_len = &msg->submsgs[submsg_wpos] - ((uint8_t *)msg);
    uint32_t dst_addr = part->metatraffic_unicast_locator.addr.udp4.addr;
    uint16_t dst_port = part->metatraffic_unicast_locator.port;
#ifdef SEDP_VERBOSE
    printf("sending %d bytes of SEDP sub catchup messages or HB to 0x%08x:%d\r\n",
           payload_len, dst_addr, dst_port);
#endif // SEDP_VERBOSE
    frudp_tx(dst_addr, dst_port, (const uint8_t *)msg, payload_len);
  }
  else
  {      
#ifdef SEDP_VERBOSE
    if (with_msgs)
    {
      printf("no SEDP pub data to send to new participant\r\n");
    }
#endif // SEDP_VERBOSE
  }
}

// currently this only gets called for SEDP messages
void frudp_publish(frudp_pub_t *pub, frudp_submsg_data_t *submsg)
{
  // TODO: for best-effort connections, don't buffer it like this
  // check to see if the publisher has a buffer or not, first....
  
  // (todo: allow people to stuff the message directly in the pub and
  // call this function with sample set to NULL to indicate this)

  // find first place we can buffer this sample
  pub->max_tx_sn_avail.low++; // TODO: care about sample counts over 32 bits
  frudp_submsg_data_t *pub_submsg = pub->data_submsgs[pub->next_submsg_idx];

  *pub_submsg = *submsg;
  if (submsg->writer_sn.low == g_frudp_sn_unknown.low) // todo: 64 bits
  {
    pub_submsg->writer_sn = pub->next_sn;
    pub->next_sn.low++; // todo: > 32 bits
  }
  memcpy(pub_submsg->data, 
         submsg->data, 
         submsg->header.len - sizeof(frudp_submsg_data_t) + 4);
  //pub_sample->data_len = sample->data_len;
  // TODO: now, send DATA and HEARTBEAT submessages
  printf("frudp publish %d bytes, seq num %d:%d\r\n",
         submsg->header.len,
         (int)pub_submsg->writer_sn.high,
         (int)pub_submsg->writer_sn.low);
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
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_pub_tx_buf);
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

  frudp_submsg_heartbeat_t *hb_submsg = 
    (frudp_submsg_heartbeat_t *)&msg->submsgs[submsg_wpos];
  hb_submsg->header.id = FRUDP_SUBMSG_ID_HEARTBEAT;
  hb_submsg->header.flags = 0x3; // todo: spell this out
  hb_submsg->header.len = 28;
  hb_submsg->reader_id = submsg->reader_id;
  hb_submsg->writer_id = submsg->writer_id;
  hb_submsg->first_sn.low = 1; // todo, should increase each time (?)
  hb_submsg->first_sn.high = 0; // todo
  hb_submsg->last_sn = hb_submsg->first_sn; //submsg->writer_sn;
  /*
  printf(" hb submsg publish last_sn = %d:%d\n",
         (int)hb_submsg->last_sn.high,
         (int)hb_submsg->last_sn.low);
  */
  static int hb_count = 0;
  hb_submsg->count = hb_count++;

  submsg_wpos += 4 + hb_submsg->header.len;

  int payload_len = &msg->submsgs[submsg_wpos] - ((uint8_t *)msg);
  frudp_tx(freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP), 
           frudp_mcast_builtin_port(),
           (const uint8_t *)msg, payload_len);
  pub->next_submsg_idx++;

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

frudp_pub_t *frudp_pub_from_writer_id(const frudp_eid_t id)
{
  //printf("pub from writer id 0x%08x\n", htonl(id.u));
  for (int i = 0; i < g_frudp_num_pubs; i++)
  {
    frudp_pub_t *p = &g_frudp_pubs[i];
    //printf("  comp %d: 0x%08x ?= 0x%08x\n",
    //       i, htonl(id.u), htonl(p->writer_id.u));
    if (id.u == p->writer_eid.u)
      return p;
  }
  return NULL;
}

void frudp_pub_rx_acknack(frudp_pub_t *pub,
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
        frudp_part_t *part = frudp_part_find(guid_prefix);
        if (!part)
        {
          printf("      woah there partner. you from around these parts?\n");
          return;
        }

        frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_pub_tx_buf);
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
        //printf("hb writer id = 0x%08x\n", htonl(data->writer_id.u));
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

void frudp_add_writer(const frudp_writer_t *writer)
{
  if (g_frudp_num_writers >= FRUDP_MAX_WRITERS)
    return;
  g_frudp_writers[g_frudp_num_writers] = *writer;
  g_frudp_num_writers++;
  printf("add_writer(%08x => ", 
         (unsigned)freertps_htonl(writer->writer_eid.u));
  frudp_print_guid(&writer->reader_guid);
  printf(")\n");
}

bool frudp_publish_user_msg_frag(
    frudp_pub_t *pub,
    const uint32_t frag_num, 
    const uint8_t *frag, 
    const uint32_t frag_len,
    const uint32_t frag_used_len,
    const uint32_t msg_len)
{
  //printf("publish frag %d : %d bytes\n", frag_num, frag_len);
  // todo: consolidate this with the non-fragmented TX function...
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_pub_user_tx_buf);
  uint16_t submsg_wpos = 0;

  if (frag_num == 1)
  {
    // craft a tx packet and stuff it
    frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
    ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
    ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
    ts_submsg->header.len = 8;
    fr_time_t t = fr_time_now();
    memcpy(ts_submsg->contents, &t, 8);
    submsg_wpos += 4 + 8;
  }
  // append the data frag submessage ////////////////////////////////////////
  frudp_submsg_data_frag_t *d =
    (frudp_submsg_data_frag_t *)&msg->submsgs[submsg_wpos];
  d->header.id = FRUDP_SUBMSG_ID_DATA_FRAG;
  d->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  d->header.len = sizeof(frudp_submsg_data_frag_t) + frag_used_len;
  d->extraflags = 0;
  d->octets_to_inline_qos = 28;
  d->writer_sn = pub->next_sn;
  if (frag_num == 1)
    pub->next_sn.low++; // todo: something smarter
  d->fragment_starting_number = frag_num;
  d->fragments_in_submessage = 1;
  d->fragment_size = frag_len;
  d->sample_size = msg_len; // + 4;
  /*
  if (frag_num == 1)
  {
    frudp_encapsulation_scheme_t *scheme =
      (frudp_encapsulation_scheme_t *)((uint8_t *)d->data);
    scheme->scheme = freertps_htons(FRUDP_SCHEME_CDR_LE);
    scheme->options = 0;
  }
  */
  uint8_t *outbound_frag_payload = (uint8_t *)&d->data[0];
  memcpy(outbound_frag_payload, frag, frag_used_len);
  submsg_wpos += d->header.len + 4;
  const int udp_payload_len = 
    (uint8_t *)&msg->submsgs[submsg_wpos] - (uint8_t *)msg;
  //printf("rtps udp payload = %d bytes\n", (int)udp_payload_len);
  // now, iterate through all matched-writers and send the message as needed
  for (int i = 0; i < g_frudp_num_writers; i++)
  {
    frudp_writer_t *w = &g_frudp_writers[i];
    if (w->writer_eid.u == pub->writer_eid.u)
    {
      // we want to send here. if we haven't already sent to the same
      // locator, update the guid and send the message
      // todo: figure out between sending to multicast and sending to unicast
      // and don't re-multicast to the same domain
      d->reader_id = w->reader_guid.eid; // todo copy here...
      d->writer_id = w->writer_eid;
      frudp_part_t *part = frudp_part_find(&w->reader_guid.prefix);
      if (!part)
        continue; // shouldn't happen; this implies inconsistency somewhere
      frudp_tx(part->default_unicast_locator.addr.udp4.addr,
               part->default_unicast_locator.port,
               (const uint8_t *)msg,
               udp_payload_len);
    }
  }
  return true;
}

bool frudp_publish_user_msg(frudp_pub_t *pub,
    const uint8_t *payload, const uint32_t payload_len)
{
  //printf("publish user msg %d bytes\n", (int)payload_len);
  if (pub->reliable)
  {
    // shouldn't be hard to push this through the reliable-comms machinery 
    // written for SEDP, but it's not done yet.
    FREERTPS_ERROR("user reliable publishing not quite done yet.\n");
    return false;
  }
  // craft a tx packet and stuff it
  frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_pub_user_tx_buf);
  fr_time_t t = fr_time_now();
  uint16_t submsg_wpos = 0;
  frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
  ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
  ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
  ts_submsg->header.len = 8;
  memcpy(ts_submsg->contents, &t, 8);
  submsg_wpos += 4 + 8;
  // now, append the data submessage ////////////////////////////////////////
  frudp_submsg_data_t *d = (frudp_submsg_data_t *)&msg->submsgs[submsg_wpos];
  d->header.id = FRUDP_SUBMSG_ID_DATA;
  d->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN |
                    FRUDP_FLAGS_DATA_PRESENT;
  d->header.len = sizeof(frudp_submsg_data_t) /*+ 4*/ + payload_len;
  d->extraflags = 0;
  d->octets_to_inline_qos = 16;
  d->writer_sn = pub->next_sn;
  frudp_encapsulation_scheme_t *scheme =
    (frudp_encapsulation_scheme_t *)((uint8_t *)d->data);
  scheme->scheme = freertps_htons(FRUDP_SCHEME_CDR_LE);
  scheme->options = 0;
  uint8_t *outbound_payload = (uint8_t *)(&d->data[4]);
  // todo: bounds checking
  /*
  printf("copying in payload:\n");
  for (int j = 0; j < payload_len; j++)
  {
    printf("%02x", (int)(((uint8_t *)payload)[j]));
    if (j % 8 == 3)
      printf(" ");
    else if (j % 8 == 7)
      printf("\n");
  }
  printf("\n");
  */

  memcpy(outbound_payload, payload, payload_len);

  //memcpy(&msg->submsgs[submsg_wpos], submsg, 4 + submsg->header.len);
  //data_submsg, submsg, 4 + submsg->header.len);
  submsg_wpos += 4 + d->header.len;

  ///////////////////////////////////////////////////////////////////////
  /*
  frudp_submsg_heartbeat_t *hb = 
    (frudp_submsg_heartbeat_t *)&msg->submsgs[submsg_wpos];
  hb->header.id = FRUDP_SUBMSG_ID_HEARTBEAT;
  hb->header.flags = 0x3; // todo: spell this out
  hb->header.len = 28;
  hb->first_sn.low = 1; // todo
  hb->first_sn.high = 0; // todo
  hb->last_sn = d->writer_sn;
  static int hb_count = 0;
  hb->count = hb_count++;

  submsg_wpos += 4 + hb->header.len;
  */

  const int udp_payload_len = 
    (uint8_t *)&msg->submsgs[submsg_wpos] - (uint8_t *)msg;
  //printf("rtps udp payload = %d bytes\n", (int)udp_payload_len);

  // now, iterate through all matched-writers and send the message as needed
  for (int i = 0; i < g_frudp_num_writers; i++)
  {
    frudp_writer_t *w = &g_frudp_writers[i];
    if (w->writer_eid.u == pub->writer_eid.u)
    {
      // we want to send here. if we haven't already sent to the same
      // locator, update the guid and send the message
      // todo: figure out between sending to multicast and sending to unicast
      // and don't re-multicast to the same domain
      d->reader_id = w->reader_guid.eid; // todo copy here...
      d->writer_id = w->writer_eid;
      frudp_part_t *part = frudp_part_find(&w->reader_guid.prefix);
      if (!part)
        continue; // shouldn't happen; this implies inconsistency somewhere
      // also, update the reader/writer ID's for the heartbeat submsg
      // actually.. i don't think we have to send heartbeats to best-effort..
      //hb->reader_id = d->reader_id;
      //hb->writer_id = d->writer_id;
      //frudp_locator_t *loc = part->default_unicast_locator;
      // todo: more than ipv4
      /*
      for (int j = 0; j < udp_payload_len; j++)
      {
        printf("%02x", (int)(((uint8_t *)msg)[j]));
        if (j % 8 == 3)
          printf(" ");
        else if (j % 8 == 7)
          printf("\n");
      }
      printf("\n");
      */
      /*
      printf("tx %d bytes to ", 
             udp_payload_len);
      frudp_print_guid(&w->reader_guid);
      printf("\r\n");
      */
      //printf("tx 
      frudp_tx(part->default_unicast_locator.addr.udp4.addr,
               part->default_unicast_locator.port,
               (const uint8_t *)msg,
               udp_payload_len);
    }
  }

  pub->next_sn.low++; // todo: 64-bit
  return true;
}

