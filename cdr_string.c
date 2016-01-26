#include "freertps/cdr_string.h"

bool fr_cdr_string_parse(char *buf, uint32_t buf_len, struct fr_cdr_string *s)
{
  int wpos = 0;
  for (; wpos < s->len && wpos < buf_len-1; wpos++)
    buf[wpos] = s->data[wpos];
  buf[wpos] = 0;
  if (wpos < buf_len - 1)
    return true;
  else
    return false; // couldn't fit entire string in buffer
}


