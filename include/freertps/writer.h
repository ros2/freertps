#ifndef WRITER_H
#define WRITER_H

#include "freertps/history_cache.h"
#include "freertps/types.h"
#include "freertps/reader_proxy.h"
#include <stdbool.h>

typedef struct writer
{
  // function pointers to:
  // new_change()
  history_cache_t hc;
  bool push_mode;
  bool reliable;
  seq_num_t last_change_seq_num;
  reader_proxy_list_t *matched_readers; // linked-list ftw
} writer_t;

writer_t *writer_create();
void writer_destroy(writer_t *w);
void writer_add_reader_proxy(reader_proxy_t *rp);

#endif
