#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertps/mem.h"

static uint32_t fr_malloc_bytes = 0;

void *fr_malloc(size_t size)
{
  fr_malloc_bytes += size;
  printf("                         malloc(%6d) total: %d\n",
      (int)size, (int)fr_malloc_bytes);
  return malloc(size);
}

void fr_free(void *ptr)
{
  free(ptr);
}
