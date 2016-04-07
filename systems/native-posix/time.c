#include "freertps/time.h"

#if defined(__linux__)
#include <time.h>
#else
#include <sys/time.h>
#include <mach/clock.h>
#include <mach/mach.h>
#endif

static uint64_t frac_sec(uint64_t nsec)
{
  // convert nanoseconds to the RTPS fractional seconds
  // FUTURE: provide faster macros for this which lose precision
  const uint64_t frac_sec_lcm = nsec * 8388608;
  return frac_sec_lcm / 1953125;
}

fr_time_t fr_time_now(void)
{
  fr_time_t now;

#if (defined __linux__)
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  now.seconds = ts.tv_sec;
  now.fraction = frac_sec(ts.tv_nsec);
#else
  clock_serv_t cclock;
  mach_timespec_t mts;

  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);

  now.seconds = mts.tv_sec;
  now.fraction = frac_sec(mts.tv_nsec);
#endif

  return now;
}
