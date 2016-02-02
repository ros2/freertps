#ifndef FREERTPS_CONTAINER_H
#define FREERTPS_CONTAINER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct fr_container; // opaque type
struct fr_container_node; // opaque type
struct fr_iterator; // forward declaration; declared in iterator.h

struct fr_container *fr_container_create();
uint32_t fr_container_len(struct fr_container *c);
bool fr_container_append(struct fr_container *c, void *data, size_t len);
struct fr_iterator fr_container_start(struct fr_container *c);

// these are not intended to be user APIs, but need to be seen by
// both the container and iterator classes. Thus, they're here (for now).
typedef enum fr_container_node_state
{   
  FR_CNS_NEW = 0, FR_CNS_VALID, FR_CNS_UNUSED, FR_CNS_READONLY
} fr_container_node_state_t;

typedef struct fr_container_node
{
  enum fr_container_node_state state;
  void *data;
  size_t data_len;
  size_t alloc_len;
  void *next;
} fr_container_node_t;


#endif
