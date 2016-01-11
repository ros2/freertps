#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertps/freertps.h"

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("usage: standalone_talk_n N DT_SECS\n");
    return 1;
  }
  printf("HELLO WORLD I WILL TALK AS REQUESTED BECAUSE I AM A ROBOT\r\n");
  const int n_msg = atoi(argv[1]);
  const double target_dt = atof(argv[2]);
  printf("sending %d messages at %.3f-second intervals\n", n_msg, target_dt);
  freertps_system_init();
  frudp_pub_t *pub = freertps_create_pub(
      "chatter", "std_msgs::msg::dds_::String_");
  frudp_disco_start();
  char msg[64] = {0};
  for (int pub_count = 0;
       pub_count < n_msg && freertps_system_ok();
       pub_count++)
  {
    frudp_listen(target_dt * 1000000);
    frudp_disco_tick();
    snprintf(&msg[4], sizeof(msg) - 4, "Hello World: %d", pub_count);
    uint32_t rtps_string_len = strlen(&msg[4]) + 1;
    uint32_t *str_len_ptr = (uint32_t *)msg;
    *str_len_ptr = rtps_string_len;
    freertps_publish(pub, (uint8_t *)msg, rtps_string_len + 4);
    printf("sending: [%s]\r\n", &msg[4]);
  }
  frudp_fini();
  return 0;
}
