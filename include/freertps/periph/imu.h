#ifndef IMU_H
#define IMU_H

#include <stdbool.h>

void imu_init();
bool imu_poll_accels(float *xyz);

#endif
