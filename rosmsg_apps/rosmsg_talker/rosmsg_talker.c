#include <stdio.h>
#include "freertps/freertps.h"
#include <string.h>
//#include "std_msgs/string.h"

int main(int argc, char **argv)
{
  printf("hello, world!\r\n");
  freertps_system_init();
  //struct std_msgs_string msg;
  /* could do this with macros, but I like an explicit function for debugging

     frudp_pub_t *pub = freertps_create_rosmsg_pub(std_msgs_string_typesupport,
                                                   "chatter")

     maybe nicer to make auto-generated functions instead:
     
     frudp_pub_t *pub = freertps_create_std_msgs_string_pub("chatter")
     or
     frudp_pub_t *pub = freertps_create_pub_std_msgs_string("chatter")
     or
     frudp_pub_t *pub = freertps_std_msgs_string_create_pub("chatter");
  */
  frudp_pub_t *pub = freertps_create_pub
                       ("chatter",
                        "std_msgs::msg::dds_::String_");
  int pub_count = 0;
  frudp_disco_start();
  while (freertps_system_ok())
  {
    frudp_listen(500000);
    frudp_disco_tick();
    char msg[256] = {0};
    snprintf(&msg[4], sizeof(msg) - 4, "Hello World: %d", pub_count++);
    uint32_t rtps_string_len = strlen(&msg[4]) + 1;
    *((uint32_t *)msg) = rtps_string_len;
    freertps_publish(pub, (uint8_t *)msg, rtps_string_len + 4);
    printf("sending: [%s]\n", &msg[4]);
  }
  frudp_fini();
  return 0;
}

