#ifndef FREERTPS_READER_PROXY_H
#define FREERTPS_READER_PROXY_H

#include "freertps/guid.h"
#include "freertps/part.h"

typedef struct fr_reader_proxy
{
  fr_guid_t remote_reader_guid;
  bool expects_inline_qos;
  fr_part_t *participant; // participant this reader proxy belongs to
} fr_reader_proxy_t;

typedef struct fr_reader_proxy_list
{
  fr_reader_proxy_t rp;
  fr_reader_proxy_t *next;
} fr_reader_proxy_list_t;

#endif

