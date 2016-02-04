#include "freertps/endpoint.h"
#include "freertps/guid.h"

void fr_endpoint_init(struct fr_endpoint *ep)
{
  ep->entity_id = g_fr_entity_id_unknown;
  ep->reliable = false;
  ep->with_key = false;
  ep->unicast_locators = NULL;
  ep->multicast_locators = NULL;
}
