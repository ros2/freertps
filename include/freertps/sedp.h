#ifndef FREERTPS_SEDP_H
#define FREERTPS_SEDP_H

#include "freertps/guid.h"
#include "freertps/participant.h"

void fr_sedp_init();
void fr_sedp_fini();

void fr_sedp_add_builtin_endpoints(struct fr_guid_prefix *prefix);

#endif
