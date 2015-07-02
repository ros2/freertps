#include <stdlib.h>
#include <stdbool.h>
#include "freertps/freertps.h"
#include "freertps/udp.h"
#include "freertps/subscription.h"

void freertps_perish_if(bool b, const char *msg)
{
  if (b)
  {
    FREERTPS_FATAL("%s\n", msg);
    exit(1);
  }
}

void freertps_create_subscription(const char *topic_name,
                                  const char *type_name,
                                  freertps_msg_cb_t msg_cb)
{
  // assume for now that we are only using UDP. in the future, this can
  // become smarter to handle when different (or multiple?) physical layer
  // are initialized
  frudp_add_user_subscription(topic_name, type_name, msg_cb);
}
