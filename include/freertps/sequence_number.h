#ifndef FREERTPS_SEQUENCE_NUMBER_H
#define FREERTPS_SEQUENCE_NUMBER_H

#include <stdint.h>

#define FR_SEQUENCE_NUMBER_UNKNOWN (-1)

typedef int64_t fr_sequence_number_t;

struct fr_sequence_number
{
  int32_t high;
  uint32_t low;
};

struct fr_sequence_number_set
{
  struct fr_sequence_number bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap[];
};

struct fr_sequence_number_set_32bits
{
  struct fr_sequence_number bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap;
};

extern const struct fr_sequence_number g_fr_sequence_number_unknown;

#endif
