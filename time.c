#include "freertps/time.h"
#include "freertps/freertps.h"
#include <limits.h>

fr_duration_t fr_time_diff(const fr_time_t *end, const fr_time_t *start)
{
  // FUTURE: this can probably be simplified.
  fr_duration_t d;
  if (end->seconds >= start->seconds)
  {
    d.fraction = end->fraction - start->fraction;
    if (end->fraction >= start->fraction)
      d.seconds = end->seconds - start->seconds;
    else
      d.seconds = end->seconds - start->seconds - 1;
  }
  else
  {
    d.fraction = start->fraction - end->fraction;
    if (end->fraction > start->fraction)
      d.seconds  = end->seconds - start->seconds + 1;
    else
      d.seconds  = end->seconds - start->seconds;
  }
  return d;
}

double fr_time_double(const fr_time_t *t)
{
  if (t->seconds >= 0)
    return t->seconds + t->fraction / (double)UINT_MAX;
  else
  {
    FREERTPS_ERROR("invalid fr_time: (%d, %u)\n", 
                   (int)t->seconds, (unsigned)t->fraction);
    return 0;
  }
    return t->seconds - t->fraction / (double)UINT_MAX;
}

double fr_duration_double(const fr_duration_t *t)
{
  if (t->seconds >= 0)
    return t->seconds + t->fraction / (double)UINT_MAX;
  else
    return t->seconds - t->fraction / (double)UINT_MAX;
}

double fr_time_now_double(void)
{
  fr_time_t t = fr_time_now();
  return fr_duration_double(&t);
}
