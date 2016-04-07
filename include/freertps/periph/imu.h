#ifndef IMU_H
#define IMU_H

#include <stdbool.h>

void imu_init(void);
bool imu_poll_accels(float *xyz);

#endif
