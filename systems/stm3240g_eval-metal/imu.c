#include "freertps/periph/imu.h"
#include <stdio.h>
#include "pin.h"
#include "metal/delay.h"

// stm3240g_eval doesn't have any IMU functions.

void imu_init(void)
{
  return;
}

bool imu_poll_accels(float *xyz)
{
  xyz[0] = xyz[1] = xyz[2] = 0;
  return true;
}

