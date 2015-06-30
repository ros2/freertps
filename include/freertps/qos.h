#ifndef FRUDP_QOS_H
#define FRUDP_QOS_H

#include "freertps/udp.h"

///////////////////////////////////////////////////////////////
#define FRUDP_QOS_RELIABILITY_KIND_BEST_EFFORT 1
#define FRUDP_QOS_RELIABILITY_KIND_RELIABLE    2

typedef struct
{
  uint32_t kind;
  frudp_duration_t max_blocking_time;
} __attribute__((packed)) frudp_qos_reliability_t;

///////////////////////////////////////////////////////////////
#define FRUDP_QOS_HISTORY_KIND_KEEP_LAST 0
#define FRUDP_QOS_HISTORY_KIND_KEEP_ALL  1

typedef struct
{
  uint32_t kind;
  uint32_t depth;
} __attribute__((packed)) frudp_qos_history_t;

///////////////////////////////////////////////////////////////
#define  FRUDP_QOS_PRESENTATION_SCOPE_TOPIC 1

typedef struct
{
  uint32_t scope;
  uint16_t coherent_access;
  uint16_t ordered_access;
} __attribute__((packed)) frudp_qos_presentation_t;


#endif
