#ifndef FREERTPS_TYPE_H
#define FREERTPS_TYPE_H

#include <stdint.h>

typedef uint32_t (*freertps_serialize_fptr_t)(
    void *msg, uint8_t *buf, uint32_t buf_size);

typedef struct freertps_type
{
  const char *rtps_typename;
  const freertps_serialize_fptr_t serialize;
} freertps_type_t;

#endif
