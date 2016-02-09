#ifndef FREERTPS_DURATION_H
#define FREERTPS_DURATION_H

#include <stdint.h>

struct fr_duration
{
  int32_t  seconds;
  uint32_t fraction;
} __attribute__((packed));

double fr_duration_double(const struct fr_duration *t);

extern const struct fr_duration g_fr_duration_zero;

#endif

