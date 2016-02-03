#ifndef FREERTPS_CONTAINER_H
#define FREERTPS_CONTAINER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct fr_container; // opaque type
struct fr_container_node; // opaque type
struct fr_iterator; // forward declaration; declared in iterator.h

#define FR_CT_ARRAY_LIST 0
#define FR_CT_LIST       1

struct fr_container *fr_container_create(const uint32_t type);
uint32_t fr_container_len(struct fr_container *c);
bool fr_container_append(struct fr_container *c, void *data, size_t len);
struct fr_iterator fr_container_start(struct fr_container *c);

// these are not intended to be user APIs, but need to be seen by
// both the container and iterator classes. Thus, they're here (for now).

#define FR_CNS_NEW      0
#define FR_CNS_VALID    1
#define FR_CNS_UNUSED   2
#define FR_CNS_READONLY 3

typedef struct fr_container_node
{
  uint32_t  state;
  void     *data;
  size_t    data_len;
  size_t    alloc_len;
  void     *next;
} fr_container_node_t;

#define FR_CANS_INVALID 0
#define FR_CANS_VALID   1

typedef struct fr_container_array_node
{
  uint32_t  length;
  void     *data;
  uint8_t  *state;
  void     *next;
} fr_container_array_node_t;

typedef struct fr_container
{
  uint32_t type;
  uint32_t element_size;
  union
  {
    struct fr_container_node *n;
    struct fr_container_array_node *a;
  } u;
} fr_container_t;

#endif
