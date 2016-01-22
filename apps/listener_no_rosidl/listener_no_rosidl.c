#include <stdio.h>
#include "freertps/freertps.h"

void chatter_cb(const void *msg)
{
  uint32_t str_len = *((uint32_t *)msg);
  char buf[128] = {0};
  for (int i = 0; i < str_len && i < sizeof(buf)-1; i++)
    buf[i] = ((uint8_t *)msg)[4+i];
  printf("I heard: [%s]\n", buf);
}

int main(int argc, char **argv)
{
  printf("hello, world!\r\n");
  freertps_system_init();
  freertps_create_sub("chatter", 
                      "std_msgs::msg::dds_::String_",
                      chatter_cb);
  fr_disco_start(); // we're alive now; announce ourselves to the world
  while (freertps_system_ok())
  {
    fr_listen(1000000);
    fr_disco_tick();
  }
  fr_fini();
  return 0;
}

