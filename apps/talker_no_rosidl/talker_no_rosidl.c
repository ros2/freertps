#include <stdio.h>
#include <string.h>
#include "freertps/freertps.h"

int main(int argc, char **argv)
{
  printf("hello, world!\r\n");
  freertps_system_init();
  frudp_pub_t *pub = freertps_create_pub(
      "chatter", "std_msgs::msg::dds_::String_");
  frudp_disco_start();
  int pub_count = 0;
  char msg[64] = {0};
  while (freertps_system_ok())
  {
    frudp_listen(500000);
    frudp_disco_tick();
    snprintf(&msg[4], sizeof(msg) - 4, "Hello World: %d", pub_count++);
    uint32_t rtps_string_len = strlen(&msg[4]) + 1;
    uint32_t *str_len_ptr = (uint32_t *)msg;
    *str_len_ptr = rtps_string_len;
    freertps_publish(pub, (uint8_t *)msg, rtps_string_len + 4);
    printf("sending: [%s]\r\n", &msg[4]);
  }
  frudp_fini();
  return 0;
}
