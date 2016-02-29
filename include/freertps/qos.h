#ifndef FR_QOS_H
#define FR_QOS_H

#include "freertps/udp.h"

///////////////////////////////////////////////////////////////
#define FR_QOS_RELIABILITY_KIND_BEST_EFFORT 1
#define FR_QOS_RELIABILITY_KIND_RELIABLE    2

struct fr_qos_reliability
{
  uint32_t kind;
  struct fr_duration max_blocking_time;
} __attribute__((packed));

///////////////////////////////////////////////////////////////
#define FR_QOS_HISTORY_KIND_KEEP_LAST 0
#define FR_QOS_HISTORY_KIND_KEEP_ALL  1

typedef struct
{
  uint32_t kind;
  uint32_t depth;
} __attribute__((packed)) fr_qos_history_t;

///////////////////////////////////////////////////////////////
#define  FR_QOS_PRESENTATION_SCOPE_TOPIC 1

typedef struct
{
  uint32_t scope;
  uint16_t coherent_access;
  uint16_t ordered_access;
} __attribute__((packed)) fr_qos_presentation_t;

#endif
