#include <stdio.h>
#include "freertps/freertps.h"
#include <string.h>
#include "sensors/imu.h"
#include "freertps/timer.h"

frudp_pub_t *g_pub = NULL;

void timer_cb()
{
  float xyz[3];
  if (!imu_poll_accels(xyz))
  {
    printf("woah! couldn't poll the imu!\r\n");
    return;
  }
  /*
  printf("imu: [%+8.3f, %+8.3f, %+8.3f]\r\n",
         xyz[0], xyz[1], xyz[2]);
  */
  if (!g_pub)
    return;
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
  freertps_timer_set_freq(1, timer_cb);

  printf("hello, world!\r\n");
  freertps_system_init();
  g_pub = freertps_create_pub
            ("chatter", "std_interfaces::msg::dds_::String_");
  while (freertps_system_ok())
  {
    frudp_listen(1000000);
    frudp_disco_tick();
    //printf("sending: [%s]\n", &msg[4]);
  }
  frudp_fini();
  return 0;
}

