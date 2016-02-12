#ifndef FREERTPS_WRITER_PROXY_H
#define FREERTPS_WRITER_PROXY_H

//#include "freertps/container.h"
#include "freertps/guid.h"
//#include "freertps/locator.h"
#include "freertps/sequence_number.h"

typedef struct fr_writer_proxy
{
  struct fr_guid remote_writer_guid;
  fr_sequence_number_t highest_sequence_number;
} fr_writer_proxy_t;

#endif
