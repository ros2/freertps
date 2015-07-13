#include "freertps/freertps.h"
#include "freertps/system.h"
#include <signal.h>

static bool g_done = false;
static void sigint_handler(int signum)
{
  g_done = true;
}

void freertps_system_init()
{
  frudp_init();
  signal(SIGINT, sigint_handler);
}

bool freertps_system_ok()
{
  return !g_done;
}
