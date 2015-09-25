#include <stdio.h>
#include "freertps/freertps.h"
#include "std_msgs/string.h"

int main(int argc, char **argv)
{
  freertps_system_init();
  frudp_pub_t *pub = freertps_create_pub
                       ("chatter",
                        std_msgs__string__type.rtps_typename);
  frudp_disco_start();

  struct std_msgs__string msg;
  char data_buf[64] = {0};
  msg.data = data_buf;

  uint8_t cdr[68] = {0};

  int pub_count = 0;
  while (freertps_system_ok())
  {
    frudp_listen(500000);
    frudp_disco_tick();
    snprintf(msg.data, sizeof(data_buf), "Hello, world! %d", pub_count++);
    int cdr_len = serialize_std_msgs__string(&msg, cdr, sizeof(cdr));
    freertps_publish(pub, cdr, cdr_len);
    printf("sending: [%s]\r\n", data_buf);
  }
  frudp_fini();
  return 0;
}

