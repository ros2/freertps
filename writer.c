#include "freertps/writer.h"
#include "freertps/mem.h"

fr_writer_t *fr_writer_create(const char *topic_name, const char *type_name)
{
  // todo: duplicate topic_name and type_name
  fr_writer_t *w = fr_malloc(sizeof(fr_writer_t));
  w->push_mode = false;
  w->reliable = false;
  w->last_change_seq_num = g_fr_seq_num_unknown;
  return w;
}

void fr_writer_destroy(fr_writer_t *w)
{
  fr_free(w);
}
