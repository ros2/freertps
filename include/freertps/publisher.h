#ifndef FREERTPS_PUBLICATION_H
#define FREERTPS_PUBLICATION_H

#include "freertps/udp.h"
#include "freertps/id.h"
#include <stdint.h>

/*
typedef struct
{
  frudp_sequence_number_t sn;
  uint32_t data_len;
  uint8_t *data;
} frudp_pub_sample_t;
*/

typedef struct
{
  const char                *topic_name;
  const char                *type_name;
  frudp_entity_id_t          writer_id;
  frudp_sequence_number_t    max_tx_sn_avail;
  frudp_sequence_number_t    min_tx_sn_avail;
  uint32_t                   num_data_submsgs;
  frudp_submsg_data_t      **data_submsgs;
  uint32_t                   next_submsg_idx;
  frudp_sequence_number_t    next_sn;
} frudp_publisher_t;

frudp_publisher_t *frudp_create_publisher(const char *topic_name,
                                          const char *type_name,
                                          const frudp_entity_id_t writer_id,
                                          frudp_submsg_data_t **data_submsgs,
                                          const uint32_t num_data_submsgs);

void frudp_publish(frudp_publisher_t *publisher, frudp_submsg_data_t *submsg);

frudp_publisher_t *frudp_publisher_from_writer_id(const frudp_entity_id_t id);

void frudp_publisher_rx_acknack(frudp_publisher_t *pub,
                                frudp_submsg_acknack_t *acknack,
                                frudp_guid_prefix_t *guid_prefix);

#endif
