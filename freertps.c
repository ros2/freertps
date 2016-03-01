#include <stdlib.h>
#include <stdbool.h>
#include "freertps/container.h"
#include "freertps/freertps.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include "freertps/participant.h"
#include "freertps/udp.h"
#include "freertps/timer.h"
#include "freertps/system.h"

struct fr_timer
{
  double period;
  double last_time;
  freertps_timer_cb_t callback;
};

/*
static freertps_timer_cb_t g_fr_timer_cb = NULL;
static double g_fr_timer_period = -1;
static double g_fr_timer_last_t = 0;
*/
struct fr_container *g_fr_timers = NULL;

bool g_freertps_init_complete;

void freertps_perish_if(bool b, const char *msg)
{
  if (b)
  {
    FREERTPS_FATAL("%s\n", msg);
    exit(1);
  }
}

#if 0

// todo: return something
void freertps_create_sub(const char *topic_name,
                         const char *type_name,
                         freertps_msg_cb_t msg_cb)
{
  // assume for now that we are only using UDP. in the future, this can
  // become smarter to handle when different (or multiple?) physical layer
  // are initialized
  fr_add_user_sub(topic_name, type_name, msg_cb);
  //g_rtps_psm.create_sub(topic_name, type
}

fr_pub_t *freertps_create_pub(const char *topic_name,
                                 const char *type_name)
{
  // assume for now that we are only using UDP. in the future, this can
  // become smarter to handle when different (or multiple?) physical layers
  // are initialized
  return fr_create_user_pub(topic_name, type_name);
}

bool freertps_publish(fr_pub_t *pub,
                      const uint8_t *msg,
                      const uint32_t msg_len)
{
  // todo: other physical layers...
  return fr_publish_user_msg(pub, msg, msg_len);
}
#endif

void freertps_init()
{
  printf("freertps_init()\n");
  g_fr_timers = fr_container_create(sizeof(struct fr_timer), 5);
  fr_participant_init();
  fr_system_init();
  fr_participant_add_default_locators();
  printf("prefix: ");
  fr_guid_print_prefix(&g_fr_participant.guid_prefix);
  printf("\n");
  fr_participant_print_locators();
  fr_participant_discovery_init();
}

bool freertps_ok()
{
  return fr_system_ok();
}

void freertps_fini()
{
  printf("freertps fini()\n");
  fr_system_fini();
  fr_participant_fini();
  fr_free(g_fr_timers);
}

void freertps_spin(const uint32_t microseconds)
{
  double t_start = fr_time_now_double();
  double t_now = t_start;
  while (t_now - t_start <= 1.0e-6 * microseconds)
  {
    // first send anything that needs to be sent
    fr_participant_send_changes();

    double t_elapsed = t_now - t_start;
    double t_remaining = (microseconds * 1.0e-6) - t_elapsed;

    for (struct fr_iterator it = fr_iterator_begin(g_fr_timers);
        it.data; fr_iterator_next(&it))
    {
      struct fr_timer *timer = it.data;
      double t_until_timeout = timer->last_time + timer->period - t_now;
      if (t_until_timeout < 0)
        t_remaining = 0;
      else if (t_until_timeout < t_remaining)
        t_remaining = t_until_timeout;
    }
    int rc = fr_system_listen_at_most((uint32_t)(t_remaining * 1000000));
    if (rc < 0)
    {
      printf("fr_system_listen_at_most() error: %d\n", rc);
      break;
    }
    if (microseconds == 0) // this is a common case from, e.g., spin_some()
      break;
    t_now = fr_time_now_double();
    for (struct fr_iterator it = fr_iterator_begin(g_fr_timers);
        it.data; fr_iterator_next(&it))
    {
      struct fr_timer *timer = it.data;
      double t_until_timeout = timer->last_time + timer->period - t_now;
      if (t_until_timeout <= 0)
      {
        timer->last_time = t_now;
        timer->callback();
      }
    }
  }
}

void freertps_add_timer(uint32_t usec, freertps_timer_cb_t cb)
{
  struct fr_timer timer;
  timer.period = usec * (1.0e-6);
  timer.callback = cb;
  timer.last_time = 0;
  fr_container_append(g_fr_timers, &timer, sizeof(timer), FR_CFLAGS_NONE);
}

