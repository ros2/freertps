#ifndef FREERTPS_ITERATOR_H
#define FREERTPS_ITERATOR_H

#include "freertps/container.h"

typedef struct fr_iterator
{
  struct fr_container_node *node;
} fr_iterator_t;

struct fr_iterator fr_iterator_next(struct fr_iterator it);

#endif
