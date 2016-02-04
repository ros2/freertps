#ifndef FREERTPS_TIME_H
#define FREERTPS_TIME_H

#include <stdint.h>
#include "freertps/duration.h"

typedef struct fr_time
{
  int32_t  seconds;
  uint32_t fraction;
} fr_time_t;

fr_time_t     fr_time_now();
fr_duration_t fr_time_diff(const fr_time_t *end, const fr_time_t *start);
double fr_time_double(const fr_time_t *t);
double fr_time_now_double(); // convenience function

#endif
