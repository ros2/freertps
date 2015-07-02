#ifndef FREERTPS_SEDP_H
#define FREERTPS_SEDP_H

#include "freertps/publisher.h"
#include "freertps/subscription.h"
#include "freertps/participant.h"

void frudp_sedp_init();
void frudp_sedp_fini();
void frudp_sedp_tick();

extern frudp_publisher_t *g_sedp_subscription_pub;
void sedp_publish_subscription(frudp_subscription_t *sub);

void sedp_add_builtin_endpoints(frudp_participant_t *part);

#endif
