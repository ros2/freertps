#ifndef FREERTPS_READER_PROXY_H
#define FREERTPS_READER_PROXY_H

#include "freertps/container.h"
#include "freertps/guid.h"
#include "freertps/locator.h"
#include "freertps/sequence_number.h"

typedef struct fr_reader_proxy
{
  struct fr_guid remote_reader_guid;
  bool expects_inline_qos;
  //struct fr_container *unicast_locators;
  //struct fr_container *multicast_locators;
  fr_sequence_number_t highest_seq_num_sent;
  fr_sequence_number_t lowest_requested_change;
  //struct fr_sequence_number_set_32bits lowest_requested_change;
  //struct fr_container *changes_for_reader; // ChangeForReader (?)
} fr_reader_proxy_t;

#endif

