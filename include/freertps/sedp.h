#ifndef FREERTPS_SEDP_H
#define FREERTPS_SEDP_H

#include "freertps/publisher.h"
#include "freertps/subscription.h"

void frudp_sedp_init();
void frudp_sedp_fini();
void frudp_sedp_tick();

extern frudp_publisher_t *g_sedp_subscription_pub;
void sedp_publish_subscription(frudp_userland_subscription_request_t *sub_req);

#endif
