#ifndef FREERTPS_ITERATOR_H
#define FREERTPS_ITERATOR_H

#include "freertps/container.h"

typedef struct fr_iterator
{
  void *data;
  size_t data_len;
  ///////////////////////////////////////
  struct fr_container *c;
  union
  {
    struct fr_container_node *n;
    struct fr_container_array_node *a;
  } u;
  uint32_t array_idx;
} fr_iterator_t;

struct fr_iterator fr_iterator_begin(struct fr_container *c);
bool fr_iterator_next(struct fr_iterator *it);

#endif
