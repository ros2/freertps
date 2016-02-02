#include "freertps/iterator.h"
#include "freertps/container.h"

struct fr_iterator fr_iterator_next(struct fr_iterator it)
{
  if (!it.node)
    return it; // garbage in, garbage out
  while (it.node->next)
  {
    it.node = it.node->next;
    if (it.node->state == FR_CNS_VALID)
      return it;
  }
  it.node = NULL;
  return it; // got to the end of the list and never found a valid node
}
