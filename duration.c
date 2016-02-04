#include "freertps/duration.h"
#include <limits.h>

const struct fr_duration g_fr_duration_zero = { .seconds = 0, .fraction = 0 };

double fr_duration_double(const struct fr_duration *t)
{
  if (t->seconds >= 0)
    return t->seconds + t->fraction / (double)UINT_MAX;
  else
    return t->seconds - t->fraction / (double)UINT_MAX;
}


