#include <stdio.h>
#include <string.h>
#include "freertps/freertps.h"
#include "std_msgs/string.h"

int main(int argc, char **argv)
{
  printf("hello, world!\r\n");
  freertps_system_init();
  // frudp_pub_t *pub = freertps_create_pub("chatter", &std_msgs__string_);
  frudp_pub_t *pub = freertps_create_pub
                       ("chatter",
                        std_msgs__string__type.rtps_typename);
  frudp_disco_start();
  int pub_count = 0;
  char data_buf[64] = {0};
  struct std_msgs__string msg;
  msg.data = data_buf;

  while (freertps_system_ok())
  {
    frudp_listen(500000);
    frudp_disco_tick();
    snprintf(msg.data, sizeof(data_buf), "Hello, world! %d", pub_count++);
    //snprintf(&msg[4], sizeof(msg) - 4, "Hello World: %d", pub_count++);
    /*
    uint32_t rtps_string_len = strlen(&msg[4]) + 1;
    *((uint32_t *)msg) = rtps_string_len;
    freertps_publish(pub, (uint8_t *)msg, rtps_string_len + 4);
    */
    printf("sending: [%s]\r\n", data_buf);//&msg[4]);
  }
  frudp_fini();
  return 0;
}

