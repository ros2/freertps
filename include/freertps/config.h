#ifndef FREERTPS_CONFIG_H
#define FREERTPS_CONFIG_H

#include <stdint.h>
#include "freertps/id.h"


// default multicast group is 239.255.0.1
#define FRUDP_DEFAULT_MCAST_GROUP 0xefff0001
#define FRUDP_DOMAIN_ID  0

#define FRUDP_MAX_SUBSCRIPTIONS 50
#define FRUDP_MAX_MATCHED_READERS 50
// is this being used anymore?
#define FRUDP_MAX_PUBLISHERS 10

#define FRUDP_MAX_TOPIC_NAME_LEN 128
#define FRUDP_MAX_TYPE_NAME_LEN  128

typedef struct
{
  frudp_guid_prefix_t guid_prefix;
  int participant_id;
  uint32_t unicast_addr;
} frudp_config_t;
extern frudp_config_t g_frudp_config;

#endif
