#include "freertps/periph/imu.h"
#include <stdio.h>

void imu_init(void)
{
  printf("native-posix fake imu init\r\n");
}

bool imu_poll_accels(float *xyz)
{
  xyz[0] = 1;
  xyz[1] = 2;
  xyz[2] = 3;
  return true;
}
