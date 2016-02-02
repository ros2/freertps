#include <stdio.h>
#include <string.h>
#include "freertps/freertps.h"

int main(int argc, char **argv)
{
  printf("hello, world!\r\n");
  freertps_init();
  fr_writer_t *w = fr_writer_create("chatter", "std_msgs::msg::dds_::String");
  /*
  fr_pub_t *pub = freertps_create_pub(
      "chatter", "std_msgs::msg::dds_::String_");
  */
  fr_discovery_start();
  int pub_count = 0;
  char msg[64] = {0};
  while (freertps_ok())
  {
    fr_listen(500000);
    fr_discovery_tick();
    snprintf(&msg[4], sizeof(msg) - 4, "Hello World: %d", pub_count++);
    uint32_t rtps_string_len = strlen(&msg[4]) + 1;
    uint32_t *str_len_ptr = (uint32_t *)msg;
    *str_len_ptr = rtps_string_len;
    //freertps_publish(pub, (uint8_t *)msg, rtps_string_len + 4);
    printf("sending: [%s]\r\n", &msg[4]);
  }
  fr_fini();
  return 0;
}
