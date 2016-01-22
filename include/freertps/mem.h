#ifndef FREERTPS_MEM_H
#define FREERTPS_MEM_H

#include <stddef.h>

void *fr_malloc(size_t size);
void  fr_free(void *ptr);

#endif
