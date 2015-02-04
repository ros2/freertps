#include <stdio.h>
#include "freertps/udp.h"
#include <signal.h>

static bool g_done = false;
void sigint_handler(int signum)
{
  g_done = true;
}

int main(int argc, char **argv)
{
  freertps_udp_init();
  signal(SIGINT, sigint_handler);
  while (!g_done)
  {
    if (!freertps_udp_listen(1000))
      break;
  }
  freertps_udp_fini();
  return 0;
}

