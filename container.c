#include <stddef.h>
#include "freertps/container.h"

// initial implementation: linked-list with a read pointer and write pointer
// the idea is that this is opaque and can be changed later...

typedef struct fr_container_node
{
  void *data;
  size_t data_len;
  void *next;
} fr_container_node_t;

typedef struct fr_container
{
  struct fr_container_node *start, *rptr, *wptr;
} fr_container_t;
