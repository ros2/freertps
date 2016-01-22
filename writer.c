#include "freertps/writer.h"
#include "freertps/mem.h"

writer_t *writer_create()
{
  writer_t *w = freertps_malloc(sizeof(writer_t));
  w->push_mode = false;
  w->reliable = false;
  w->last_change_seq_num = g_freertps_seq_num_unknown;
  return w;
}

void writer_destroy(writer_t *w)
{
  freertps_free(w);
}
