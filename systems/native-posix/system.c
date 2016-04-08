#include "freertps/freertps.h"
#include "freertps/system.h"
#include <stdlib.h>
#if defined(__linux__)
#include <malloc.h>
#endif
#include <execinfo.h>
#include "freertps/psm.h"

struct rtps_psm g_rtps_psm;

/*
#include <signal.h>
static bool g_done = false;
static void sigint_handler(int signum)
{
  g_done = true;
}
*/

//#define MALLOC_DEBUG

#ifdef MALLOC_DEBUG
// save the glibc hook; we'll use it later
static void *(*g_freertps_prev_malloc_hook)(size_t, const void *); 

static void *freertps_malloc(size_t size, const void *caller)
{
  //puts("malloc\n");
  //backtrace_buffer[0] = caller;
  static const int MAX_DEPTH = 10;
  void *backtrace_buffer[MAX_DEPTH];
  void *frames[MAX_DEPTH];
  __malloc_hook = g_freertps_prev_malloc_hook;
  int stack_depth = backtrace_symbols(backtrace_buffer, MAX_DEPTH);
  char **function_names = backtrace_symbols(backtrace_buffer, MAX_DEPTH);
  printf("malloc(%u) called from %s [%p]\n", 
         (unsigned)size, function_names[0], backtrace_buffer[0]);
  free(function_names);
  void *mem = malloc(size);
  __malloc_hook = freertps_malloc;
  return mem;
}

void freertps_init_malloc_hook(void)
{
  g_freertps_prev_malloc_hook = __malloc_hook;
  __malloc_hook = freertps_malloc;
}
void (*volatile __malloc_initialize_hook)(void) = freertps_init_malloc_hook;
#endif

void freertps_system_init(void)
{
  //__malloc_hook = freertps_malloc;
  /*
  if (getenv("FREERTPS_SERIAL_GROUP"))
  {
    // udpv4 multicast group of the "simulated serial comms" group
    const char *serial_group = getenv("FREERTPS_SERIAL_GROUP");
    rtps_ser_init(serial_group);
  }
  else
    */
    g_rtps_psm = g_rtps_psm_udp;

  g_rtps_psm.init();

  //signal(SIGINT, sigint_handler); // let ROS2 handle this now
  g_freertps_init_complete = true;
}

bool freertps_system_ok(void)
{
  return true; //!g_done;
}
