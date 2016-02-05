#ifndef FREERTPS_READER_LOCATOR_H
#define FREERTPS_READER_LOCATOR_H

#include "freertps/locator.h"

typedef struct fr_reader_locator
{
  struct fr_locator locator;
  struct fr_container *unsent_changes;
  struct fr_container *requested_changes;
} fr_reader_locator_t;

#endif
