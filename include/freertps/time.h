#ifndef FREERTPS_TIME_H
#define FREERTPS_TIME_H

#include <stdint.h>

typedef struct
{
  int32_t  seconds;
  uint32_t fraction;
} fr_time_t;

typedef fr_time_t fr_duration_t;

fr_time_t     fr_time_now(void);
fr_duration_t fr_time_diff(const fr_time_t *end, const fr_time_t *start);
double fr_time_double(const fr_time_t *t);
double fr_time_now_double(void); // convenience function
double fr_duration_double(const fr_duration_t *t);

#endif
