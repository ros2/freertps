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
  frudp_init();
  signal(SIGINT, sigint_handler);
  while (!g_done)
  {
    if (!frudp_listen(1000))
      break;
    frudp_discovery_tick();
  }
  frudp_fini();
  return 0;
}

