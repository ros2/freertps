#ifndef FR_PUB_H
#define FR_PUB_H

#include "freertps/udp.h"
#include "freertps/id.h"
#include "freertps/config.h"
#include "freertps/part.h"
#include <stdint.h>

typedef struct
{
  const char        *topic_name;
  const char        *type_name;
  fr_eid_t           writer_eid;
  fr_seq_num_t       max_tx_sn_avail;
  fr_seq_num_t       min_tx_sn_avail;
  uint32_t           num_data_submsgs;
  fr_submsg_data_t **data_submsgs;
  uint32_t           next_submsg_idx;
  fr_seq_num_t       next_sn;
  bool               reliable;
} fr_pub_t;

extern fr_pub_t g_fr_pubs[FR_MAX_PUBS];
extern uint32_t g_fr_num_pubs;

//////////////////////////////////////////////////////////////

typedef struct
{
  fr_guid_t reader_guid;
  fr_eid_t writer_eid;
} fr_writer_t; // currently only supports best-effort connections

extern fr_writer_t g_fr_writers[FR_MAX_WRITERS];
extern uint32_t g_fr_num_writers;

//////////////////////////////////////////////////////////////

fr_pub_t *fr_create_pub(const char *topic_name,
                        const char *type_name,
                        const fr_eid_t writer_id,
                        fr_submsg_data_t **data_submsgs,
                        const uint32_t num_data_submsgs);

void fr_publish(fr_pub_t *publication, fr_submsg_data_t *submsg);

bool fr_publish_user_msg(fr_pub_t *pub,
    const uint8_t *msg, const uint32_t msg_len);

bool fr_publish_user_msg_frag(
    fr_pub_t *publication,
    const uint32_t frag_num, 
    const uint8_t *frag, 
    const uint32_t frag_len,
    const uint32_t frag_used_len,
    const uint32_t msg_len);

fr_pub_t *fr_pub_from_writer_id(const fr_eid_t id);

void fr_pub_rx_acknack(fr_pub_t *pub,
                       fr_submsg_acknack_t *acknack,
                       fr_guid_prefix_t *guid_prefix);

fr_pub_t *fr_create_user_pub(const char *topic_name,
                             const char *type_name);

void fr_add_writer(const fr_writer_t *writer);

void fr_send_sedp_msgs(fr_part_t *part);

#endif
