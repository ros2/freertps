#include "freertps/time.h"
#include <time.h>

fr_time_t fr_time_now()
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  // convert nanoseconds to the RTPS fractional seconds
  // FUTURE: provide faster macros for this which lose precision
  const uint64_t frac_sec_lcm = (uint64_t)ts.tv_nsec * 8388608;
  const uint64_t frac_sec = frac_sec_lcm / 1953125;
  fr_time_t now;
  now.seconds = ts.tv_sec;
  now.fraction = frac_sec;
  return now;
}
