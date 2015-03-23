#include "freertps/time.h"
#include <limits.h>
#include <math.h>

void run_test(const int32_t t1_secs, const uint32_t t1_frac,
              const int32_t t2_secs, const uint32_t t2_frac)
{
  const fr_time_t t1 = { .seconds = t1_secs, .fraction = t1_frac };
  const fr_time_t t2 = { .seconds = t2_secs, .fraction = t2_frac };
  const fr_duration_t d = fr_time_diff(&t1, &t2);
  const double d_d = fr_duration_double(&d);

  const double t1_d = fr_time_double(&t1);
  const double t2_d = fr_time_double(&t2);
  const double diff_d = t1_d - t2_d;

  if (fabs(d_d - diff_d) > 1e-9)
    printf("FAILED! %.9f != %.9f\n", d_d, diff_d);
  printf("(%d, %u) - (%d, %u) = %.3f - %.3f = %.3f (%d, %u)\n", 
         t1.seconds, t1.fraction,
         t2.seconds, t2.fraction,
         t1_d, t2_d, d_d,
         d.seconds, d.fraction);
}

int main(int argc, char **argv)
{
  run_test(7, 100000000, 5,  50000000);
  run_test(7, 100000000, 5, 150000000);
  run_test(5, 100000000, 7,  50000000);
  run_test(5, 100000000, 7, 150000000);
  run_test(5, 0, 7, 0);
  run_test(7, 0, 5, 0);
  return 0;
}

