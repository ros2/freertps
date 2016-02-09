#ifndef FREERTPS_TIME_H
#define FREERTPS_TIME_H

#include <stdint.h>
#include "freertps/duration.h"

struct fr_time
{
  int32_t  seconds;
  uint32_t fraction;
};

struct fr_time fr_time_now();
struct fr_duration fr_time_diff(const struct fr_time *end,
    const struct fr_time *start);
double fr_time_double(const struct fr_time *t);
double fr_time_now_double(); // convenience function

#endif
