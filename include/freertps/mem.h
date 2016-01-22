#ifndef FREERTPS_MEM_H
#define FREERTPS_MEM_H

#include <stddef.h>

void *freertps_malloc(size_t size);
void freertps_free(void *ptr);

#endif
