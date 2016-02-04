#ifndef FREERTPS_CONTAINER_H
#define FREERTPS_CONTAINER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "freertps/rc.h"

struct fr_container; // opaque type
struct fr_container_node; // opaque type
struct fr_iterator; // forward declaration; declared in iterator.h

#define FR_CT_ARRAY_LIST 0
#define FR_CT_LIST       1

#define FR_CFLAGS_NONE           0
#define FR_CFLAGS_TAKE_OWNERSHIP 1

struct fr_container *fr_container_create(
    const uint32_t element_size, uint32_t array_block_length);
void fr_container_free(struct fr_container *c);

uint32_t fr_container_len(struct fr_container *c);
fr_rc_t fr_container_append(
    struct fr_container *c, void *data, size_t len, uint32_t flags);

// these are not intended to be user APIs, but need to be seen by
// both the container and iterator classes. Thus, they're here (for now).

#define FR_CNS_NEW      0
#define FR_CNS_VALID    1
#define FR_CNS_UNUSED   2

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
  uint8_t  *state; // convenience pointer to the last bytes of the data block
  void     *next;
  uint8_t   data[];
} fr_container_array_node_t;

typedef struct fr_container
{
  uint32_t type;
  uint8_t  data[]; // placeholder to make sure we never allocate one directly
} fr_container_t;

typedef struct fr_container_list
{
  uint32_t type;
  struct fr_container_node *head;
} fr_container_list_t;

typedef struct fr_container_array_list
{
  uint32_t type;
  uint32_t element_size;
  struct fr_container_array_node head;
} fr_container_array_list_t;

#endif
