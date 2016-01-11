#include <stdio.h>
#include <stdlib.h>
#include "freertps/freertps.h"

static int n_msg_recv = 0; // cue the raptors plz

void chatter_cb(const void *msg)
{
  uint32_t str_len = *((uint32_t *)msg);
  char buf[128] = {0};
  for (int i = 0; i < str_len && i < sizeof(buf)-1; i++)
    buf[i] = ((uint8_t *)msg)[4+i];
  printf("I heard: [%s]\n", buf);
  n_msg_recv++;
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("usage: standalone_listen_for_n N TIMEOUT_SECONDS\n");
    return 1;
  }
  const int target_n_msg_recv = atoi(argv[1]);
  const double max_seconds = atof(argv[2]);
  fr_time_t t_start = fr_time_now();
  freertps_system_init();
  freertps_create_sub("chatter", 
                      "std_msgs::msg::dds_::String_",
                      chatter_cb);
  frudp_disco_start(); // we're alive now; announce ourselves to the world
  while (freertps_system_ok())
  {
    frudp_listen(100000);
    frudp_disco_tick(); // stayin' alive FLOOR-TOM FLOOR-TOM CYMBAL-CRASH
    fr_time_t t = fr_time_now();
    fr_duration_t dt = fr_time_diff(&t, &t_start);
    double dt_secs = fr_duration_double(&dt);
    if (n_msg_recv >= target_n_msg_recv ||
        dt_secs > max_seconds)
      break;
  }
  frudp_fini();
  bool success = n_msg_recv >= target_n_msg_recv;
  if (success)
    printf("HOORAY, I received all the messages.\n");
  else
    printf("SADNESS, I didn't receive all the messages.\n");
  return success ? 0 : 1;
}

