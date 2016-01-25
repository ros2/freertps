#ifndef FREERTPS_CONTAINER_H
#define FREERTPS_CONTAINER_H

#define <stdbool.h>

#define CONTAINER_LIST 1

struct fr_container_t; // opaque type

struct fr_container_t *fr_container_create(int container_type);
bool  fr_container_resize(struct fr_container_t *c, size_t size);
void *fr_container_get(struct fr_container_t *c, size_t idx);
bool fr_container_set(struct fr_container_t  *c, size_t idx, void *val);

#endif
