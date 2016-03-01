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
  freertps_init();
  fr_reader_t *r = fr_reader_create("chatter", "std_msgs::msg::dds_::String_",
      FR_READER_TYPE_BEST_EFFORT);
  r->endpoint.entity_id.s.kind = 0x4;
  r->endpoint.entity_id.s.key[0] = 1;
  r->endpoint.entity_id.s.key[1] = 2;
  r->endpoint.entity_id.s.key[2] = 3;
  r->msg_rx_cb = chatter_cb;
  fr_participant_add_reader(r);
  while (freertps_ok())
    freertps_spin(500000);
  freertps_fini();
  return 0;
}

