#include <stdio.h>
#include "freertps/freertps.h"
#include <string.h>
#include "sensors/imu.h"
#include "freertps/timer.h"

frudp_pub_t *g_pub = NULL;

typedef struct
{
  int32_t sec;
  uint32_t nanosec;
} __attribute__((packed)) builtin_interfaces__time_t;

typedef struct
{
  builtin_interfaces__time_t stamp;
  uint32_t frame_id_len_;
  char frame_id[];
} __attribute__((packed)) std_interfaces__header_t;

typedef struct
{
  double x;
  double y;
  double z;
} __attribute__((packed)) geometry_msgs__vector3_t;

typedef struct
{
  double x, y, z, w;
} __attribute__((packed)) geometry_msgs__quaternion_t;

typedef struct
{
  geometry_msgs__quaternion_t orientation;
  double orientation_covariance[9];
  geometry_msgs__vector3_t angular_velocity;
  double angular_velocity_covariance[9];
  geometry_msgs__vector3_t linear_acceleration;
  double linear_acceleration_covariance[9];
} __attribute__((packed)) sensor_interfaces__imu_t;

void timer_cb()
{
  static float xyz[3];
  if (!imu_poll_accels(xyz))
  {
    printf("woah! couldn't poll the imu!\r\n");
    return;
  }
  //printf("imu: [%+8.3f, %+8.3f, %+8.3f]\r\n",
  //       xyz[0], xyz[1], xyz[2]);
  if (!g_pub)
    return;
  static char __attribute__((aligned(4))) msg[512] = {0};
  std_interfaces__header_t *header = (std_interfaces__header_t *)msg;
  header->stamp.sec = 1234;
  header->stamp.nanosec = 5678;
  static const char *frame_id = "imu_frame12";
  header->frame_id_len_ = strlen(frame_id) + 1;
  memcpy(header->frame_id, frame_id, header->frame_id_len_);
  static sensor_interfaces__imu_t imu;
  static int pub_count = 0;
  pub_count++;
  imu.orientation.x = 1 + pub_count;
  imu.orientation.y = 2;
  imu.orientation.z = 3;
  imu.orientation.w = 4;
  imu.angular_velocity.x = 5;
  imu.angular_velocity.y = 6;
  imu.angular_velocity.z = 7;
  imu.linear_acceleration.x = xyz[0];
  imu.linear_acceleration.y = xyz[1];
  imu.linear_acceleration.z = xyz[2];
  for (int i = 0; i < 9; i++)
  {
    imu.orientation_covariance[i] = 11 + i;
    imu.angular_velocity_covariance[i] = 20 + i;
    imu.linear_acceleration_covariance[i] = 29 + i;
  }
  uint8_t *wpos = (uint8_t *)(header->frame_id) + header->frame_id_len_; // + 1;
  memcpy(wpos, &imu, sizeof(imu));
  uint32_t header_len = (wpos - (uint8_t *)msg);
  uint32_t cdr_len = sizeof(imu) + header_len;
  //printf("cdr len = %d header_len = %d\n", (int)cdr_len, (int)header_len);
  freertps_publish(g_pub, (uint8_t *)msg, cdr_len);
  //printf("pub\n");
  /*
  static char __attribute__((aligned(4))) msg[256] = {0};
  static int pub_count = 0;
  snprintf(&msg[4], sizeof(msg) - 4, "Hello World: %d", pub_count++);
  uint32_t rtps_string_len = strlen(&msg[4]) + 1;
  *((uint32_t *)msg) = rtps_string_len;
  freertps_publish(g_pub, (uint8_t *)msg, rtps_string_len + 4);
  */
}

int main(int argc, char **argv)
{
  imu_init();
  freertps_system_init();
  freertps_timer_set_freq(10, timer_cb);
  //freertps_timer_set_freq(1000, timer_cb);
  printf("hello, world!\r\n");
  g_pub = freertps_create_pub
            ("imu", "sensor_msgs::msg::dds_::Imu_");
  frudp_disco_start();
  while (freertps_system_ok())
  {
    frudp_listen(1000000);
    frudp_disco_tick();
  }
  frudp_fini();
  return 0;
}

