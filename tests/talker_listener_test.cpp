#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gtest/gtest.h"

#include <freertps/freertps.h>

// necessary due to limitations of the API
static uint32_t n_msg_recv = 0; // cue the raptors plz

void chatter_cb(const void *msg)
{
  uint32_t str_len = *((uint32_t *)msg);
  char buf[128] = {0};
  for (uint32_t i = 0; i < str_len && i < sizeof(buf)-1; i++)
    buf[i] = ((uint8_t *)msg)[4+i];
  printf("I heard: [%s]\n", buf);
  n_msg_recv++;
}

TEST(talker_listener_test, talk_n_times)
{
  const int n_msg = 15;
  const double max_seconds = 30;
  const double target_dt = 0.1;

  fr_time_t t_start = fr_time_now();
  pid_t pid = fork();

  if (pid == 0)
  {
    // child process talks

    printf("HELLO WORLD I WILL TALK AS REQUESTED BECAUSE I AM A ROBOT\r\n");
    printf("sending %d messages at %.3f-second intervals\n", n_msg, target_dt);
    freertps_system_init();
    frudp_pub_t *pub = freertps_create_pub(
        "chatter", "std_msgs::msg::dds_::String_");
    frudp_disco_start();
    char msg[64] = {0};
    for (uint32_t pub_count = 0;
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
    return;
  }

  freertps_system_init();
  // parent process listens
  freertps_create_sub("chatter",
                      "std_msgs::msg::dds_::String_",
                      chatter_cb);
  frudp_disco_start(); // we're alive now; announce ourselves to the world
  while (freertps_system_ok())
  {
    frudp_listen(target_dt * 1000000);
    frudp_disco_tick(); // stayin' alive FLOOR-TOM FLOOR-TOM CYMBAL-CRASH
    fr_time_t t = fr_time_now();
    fr_duration_t dt = fr_time_diff(&t, &t_start);
    double dt_secs = fr_duration_double(&dt);
    if (n_msg_recv >= n_msg ||
        dt_secs > max_seconds)
      break;
  }
  frudp_fini();
  ASSERT_GE(n_msg_recv, n_msg);
  printf("HOORAY, I received all the messages.\n");
}
