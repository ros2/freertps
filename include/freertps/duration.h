#ifndef FREERTPS_DURATION_H
#define FREERTPS_DURATION_H

#include <stdint.h>

typedef struct fr_duration
{
  int32_t  seconds;
  uint32_t fraction;
} fr_duration_t;

double fr_duration_double(const struct fr_duration *t);

extern const struct fr_duration g_fr_duration_zero;

#endif

