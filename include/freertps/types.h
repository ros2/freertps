#ifndef FREERTPS_TYPES_H
#define FREERTPS_TYPES_H

#include <stdint.h>

typedef struct
{
  int32_t high;
  uint32_t low;
} seq_num_t; // sequence number
extern const seq_num_t g_freertps_seq_num_unknown;

typedef struct
{
  seq_num_t bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap[];
} seq_num_set_t;

typedef struct
{
  seq_num_t bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap;
} seq_num_set_32bits_t;

#endif
