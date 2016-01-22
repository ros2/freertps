#ifndef FREERTPS_READER_PROXY_H
#define FREERTPS_READER_PROXY_H

typedef struct reader_proxy_list
{
  frudp_guid_t remote_reader_guid;
  bool expects_inline_qos;
  frudp_part_t *participant; // participant this reader proxy belongs to
  struct reader_proxy *next; // next element in list
} reader_proxy_list_t;

#endif
