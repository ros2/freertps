#ifndef FREERTPS_READER_LOCATOR_H
#define FREERTPS_READER_LOCATOR_H

#include "freertps/locator.h"
#include "freertps/sequence_number.h"

typedef struct fr_reader_locator
{
  bool expects_inline_qos;
  struct fr_locator locator;
  fr_sequence_number_t highest_seq_num_sent;
  fr_sequence_number_t lowest_requested_change;
} fr_reader_locator_t;

struct fr_reader_locator *fr_reader_locator_create();
void fr_reader_locator_init(struct fr_reader_locator *loc);

#endif
