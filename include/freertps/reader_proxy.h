#ifndef FREERTPS_READER_PROXY_H
#define FREERTPS_READER_PROXY_H

#include "freertps/guid.h"
#include "freertps/participant.h"

typedef struct fr_reader_proxy
{
  fr_guid_t remote_reader_guid;
  bool expects_inline_qos;
  struct fr_participant *participant; // participant this reader proxy belongs to
} fr_reader_proxy_t;

typedef struct fr_reader_proxy_list
{
  fr_reader_proxy_t rp;
  fr_reader_proxy_t *next;
} fr_reader_proxy_list_t;

#endif

