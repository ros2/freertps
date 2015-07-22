#include <stdlib.h>
#include <stdbool.h>
#include "freertps/freertps.h"
#include "freertps/udp.h"
#include "freertps/sub.h"

void freertps_perish_if(bool b, const char *msg)
{
  if (b)
  {
    FREERTPS_FATAL("%s\n", msg);
    exit(1);
  }
}

void freertps_create_sub(const char *topic_name,
                         const char *type_name,
                         freertps_msg_cb_t msg_cb)
{
  // assume for now that we are only using UDP. in the future, this can
  // become smarter to handle when different (or multiple?) physical layer
  // are initialized
  frudp_add_user_sub(topic_name, type_name, msg_cb);
}

frudp_pub_t *freertps_create_pub(const char *topic_name,
                                 const char *type_name)
{
  // assume for now that we are only using UDP. in the future, this can
  // become smarter to handle when different (or multiple?) physical layers
  // are initialized
  return frudp_create_user_pub(topic_name, type_name);
}

bool freertps_publish(frudp_pub_t *pub,
                      const uint8_t *msg,
                      const uint32_t msg_len)
{
  return true;
}
