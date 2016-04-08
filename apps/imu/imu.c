#include <stdio.h>
#include "freertps/freertps.h"
#include <string.h>
#include "freertps/periph/imu.h"
#include "freertps/timer.h"
#include "sensor_msgs/imu.h"

frudp_pub_t *g_pub = NULL;

void timer_cb(void)
{
  if (!g_pub)
    return; // startup race condition. not ready yet. hold your horses. wait.

  static float xyz[3];
  if (!imu_poll_accels(xyz))
  {
    printf("woah! couldn't poll the imu!\r\n");
    return;
  }
  //printf("imu: [%+8.3f, %+8.3f, %+8.3f]\r\n",
  //       xyz[0], xyz[1], xyz[2]);
  static int pub_count = 0;
  pub_count++;

  static struct sensor_msgs__imu imu_msg;
  static char *g_frame_id = "imu_frame";
 
  imu_msg.header.stamp.sec = 1234;
  imu_msg.header.stamp.nanosec = 5678;
  imu_msg.header.frame_id = g_frame_id;
  imu_msg.orientation.x = 1 + pub_count;
  imu_msg.orientation.y = 2;
  imu_msg.orientation.z = 3;
  imu_msg.orientation.w = 4;
  imu_msg.angular_velocity.x = 5;
  imu_msg.angular_velocity.y = 6;
  imu_msg.angular_velocity.z = 7;
  imu_msg.linear_acceleration.x = xyz[0];
  imu_msg.linear_acceleration.y = xyz[1];
  imu_msg.linear_acceleration.z = xyz[2];
  for (int i = 0; i < 9; i++)
  {
    imu_msg.orientation_covariance[i] = 11 + i;
    imu_msg.angular_velocity_covariance[i] = 20 + i;
    imu_msg.linear_acceleration_covariance[i] = 29 + i;
  }

  static uint8_t __attribute__((aligned(4))) cdr[512] = {0};
  uint32_t cdr_len = serialize_sensor_msgs__imu(&imu_msg, cdr, sizeof(cdr));
  freertps_publish(g_pub, (uint8_t *)cdr, cdr_len);
}

int main(int argc, char **argv)
{
  imu_init();
  freertps_system_init();
  freertps_timer_set_freq(10, timer_cb);
  //freertps_timer_set_freq(1000, timer_cb);
  printf("hello, world!\r\n");
  g_pub = freertps_create_pub("imu", sensor_msgs__imu__type.rtps_typename);
  frudp_disco_start();
  while (freertps_system_ok())
  {
    frudp_listen(1000000);
    frudp_disco_tick();
  }
  frudp_fini();
  return 0;
}

