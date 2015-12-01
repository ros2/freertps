#ifndef FREERTPS_TYPE_H
#define FREERTPS_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t (*freertps_serialize_fptr_t)(
    void *msg, uint8_t *buf, uint32_t buf_size);

typedef struct freertps_type
{
  const char *rtps_typename;
  const freertps_serialize_fptr_t serialize;
} freertps_type_t;

#define FREERTPS_ARRAY(STRUCT_NAME, TYPE_NAME) \
  typedef struct freertps__ ## STRUCT_NAME ## __array \
  { \
    TYPE_NAME * data;  \
    uint32_t size;     \
    uint32_t capacity; \
  } freertps__ ## STRUCT_NAME ## __array_t;

FREERTPS_ARRAY(bool, bool);
FREERTPS_ARRAY(byte, uint8_t);
FREERTPS_ARRAY(char, int8_t);
FREERTPS_ARRAY(uint8, uint8_t);
FREERTPS_ARRAY(int8, int8_t);
FREERTPS_ARRAY(uint16, uint16_t);
FREERTPS_ARRAY(int16, int16_t);
FREERTPS_ARRAY(uint32, uint32_t);
FREERTPS_ARRAY(int32, int32_t);
FREERTPS_ARRAY(uint64, uint64_t);
FREERTPS_ARRAY(int64, int64_t);
FREERTPS_ARRAY(float32, float);
FREERTPS_ARRAY(float64, double);
FREERTPS_ARRAY(string, char *);

#endif
