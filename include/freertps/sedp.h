#ifndef FREERTPS_SEDP_H
#define FREERTPS_SEDP_H

#include "freertps/pub.h"
#include "freertps/sub.h"
#include "freertps/part.h"

void frudp_sedp_init(void);
void frudp_sedp_start(void);
void frudp_sedp_tick(void);
void frudp_sedp_fini(void);

extern frudp_pub_t *g_sedp_sub_pub;
void sedp_publish_sub(frudp_sub_t *sub);
void sedp_publish_pub(frudp_pub_t *pub);

void sedp_add_builtin_endpoints(frudp_part_t *part);

extern frudp_pub_t *g_sedp_sub_pub;
extern frudp_pub_t *g_sedp_pub_pub;

#endif
