#include "freertps/time.h"
#include "freertps/freertps.h"
#include <limits.h>

struct fr_duration fr_time_diff(const struct fr_time *end,
    const struct fr_time *start)
{
  // FUTURE: this can probably be simplified.
  struct fr_duration d;
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

double fr_time_double(const struct fr_time *t)
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

double fr_time_now_double()
{
  struct fr_time t = fr_time_now();
  return fr_time_double(&t);
}
