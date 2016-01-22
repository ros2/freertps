#include "freertps/mem.h"
#include <stdlib.h>

void *fr_malloc(size_t size)
{
  // todo: we can add some (optional) instrumentation here
  return malloc(size);
}

void fr_free(void *ptr)
{
  // todo: we can add some (optional) instrumentation here
  free(ptr);
}
