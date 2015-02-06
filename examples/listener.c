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
  fu_init();
  signal(SIGINT, sigint_handler);
  while (!g_done)
  {
    if (!fu_listen(1000))
      break;
  }
  fu_fini();
  return 0;
}

