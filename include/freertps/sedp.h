#ifndef FREERTPS_SEDP_H
#define FREERTPS_SEDP_H

#include "freertps/guid.h"
#include "freertps/participant.h"

void fr_sedp_init();
void fr_sedp_start();
void fr_sedp_tick();
void fr_sedp_fini();

void fr_sedp_add_builtin_endpoints(struct fr_guid_prefix *prefix);

/*
extern fr_pub_t *g_sedp_sub_pub;
void sedp_publish_sub(fr_sub_t *sub);
void sedp_publish_pub(fr_pub_t *pub);

extern fr_pub_t *g_sedp_sub_pub;
extern fr_pub_t *g_sedp_pub_pub;
*/

#endif
