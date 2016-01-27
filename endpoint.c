#include "freertps/endpoint.h"
#include "freertps/guid.h"

void endpoint_init(struct fr_endpoint *ep)
{
  fr_guid_set_zero(&ep->guid);
  ep->reliable = false;
  ep->with_key = false;
  ep->unicast_locators = NULL;
  ep->multicast_locators = NULL;
}
