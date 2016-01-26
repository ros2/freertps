#ifndef FREERTPS_WRITER_PROXY_H
#define FREERTPS_WRITER_PROXY_H

#include "freertps/container.h"
#include "freertps/guid.h"
#include "freertps/locator.h"

typedef struct fr_writer_proxy
{
  struct fr_guid remote_writer_guid;
  struct fr_container_t *unicast_locators;   // for stateful reading
  struct fr_container_t *multicast_locators; // for stateful reading
} fr_writer_proxy_t;

#endif
