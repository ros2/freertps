#ifndef FREERTPS_TIME_H
#define FREERTPS_TIME_H

#include "freertps/freertps.h"
#include <stdint.h>

typedef struct
{
  int32_t  seconds;
  uint32_t fraction;
} fr_time_t;

typedef fr_time_t fr_duration_t;

fr_time_t     fr_time_now();
fr_duration_t fr_time_diff(const fr_time_t *start, const fr_time_t *end);
double fr_time_double(const fr_time_t *t);
double fr_duration_double(const fr_duration_t *t);

#endif
