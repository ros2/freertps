#ifndef FREERTPS_PUB_H
#define FREERTPS_PUB_H

#include "freertps/udp.h"
#include "freertps/id.h"
#include "freertps/config.h"
#include "freertps/part.h"
#include <stdint.h>

typedef struct
{
  const char           *topic_name;
  const char           *type_name;
  frudp_eid_t           writer_eid;
  frudp_sn_t            max_tx_sn_avail;
  frudp_sn_t            min_tx_sn_avail;
  uint32_t              num_data_submsgs;
  frudp_submsg_data_t **data_submsgs;
  uint32_t              next_submsg_idx;
  frudp_sn_t            next_sn;
  bool                  reliable;
} frudp_pub_t;

extern frudp_pub_t g_frudp_pubs[FRUDP_MAX_PUBS];
extern uint32_t g_frudp_num_pubs;

//////////////////////////////////////////////////////////////

typedef struct
{
  frudp_guid_t reader_guid;
  frudp_eid_t writer_eid;
} frudp_writer_t; // currently only supports best-effort connections

extern frudp_writer_t g_frudp_writers[FRUDP_MAX_WRITERS];
extern uint32_t g_frudp_num_writers;

//////////////////////////////////////////////////////////////

frudp_pub_t *frudp_create_pub(const char *topic_name,
                              const char *type_name,
                              const frudp_eid_t writer_id,
                              frudp_submsg_data_t **data_submsgs,
                              const uint32_t num_data_submsgs);

void frudp_publish(frudp_pub_t *publication, 
                   frudp_submsg_data_t *submsg);

bool frudp_publish_user_msg(frudp_pub_t *publication,
    const uint8_t *msg, const uint32_t msg_len);

bool frudp_publish_user_msg_frag(
    frudp_pub_t *publication,
    const uint32_t frag_num, 
    const uint8_t *frag, 
    const uint32_t frag_len,
    const uint32_t frag_used_len,
    const uint32_t msg_len);

frudp_pub_t *frudp_pub_from_writer_id(const frudp_eid_t id);

void frudp_pub_rx_acknack(frudp_pub_t *pub,
                          frudp_submsg_acknack_t *acknack,
                          frudp_guid_prefix_t *guid_prefix);

frudp_pub_t *frudp_create_user_pub(const char *topic_name,
                                   const char *type_name);

void frudp_add_writer(const frudp_writer_t *writer);

void frudp_send_sedp_hb(frudp_part_t *part, bool with_msgs);

#endif
