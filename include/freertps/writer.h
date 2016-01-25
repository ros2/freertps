#ifndef FREERTPS_WRITER_H
#define FREERTPS_WRITER_H

#include "freertps/history_cache.h"
#include "freertps/types.h"
#include "freertps/reader_proxy.h"
#include <stdbool.h>

typedef struct fr_writer
{
  // function pointers to:
  // new_change()
  fr_history_cache_t hc;
  bool push_mode;
  bool reliable;
  fr_seq_num_t last_change_seq_num;
  fr_reader_proxy_list_t *matched_readers; // linked-list ftw
} fr_writer_t;

fr_writer_t *fr_writer_create(const char *topic_name, const char *type_name);
void fr_writer_destroy(fr_writer_t *w);
void fr_writer_add_reader_proxy(fr_reader_proxy_t *rp);

#endif
