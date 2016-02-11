#include <string.h>
#include "freertps/bswap.h"
#include "freertps/encapsulation_scheme.h"
#include "freertps/parameter.h"

void fr_parameter_list_init(
    struct fr_parameter_list *s)
{
  struct fr_encapsulation_scheme *scheme = 
      (struct fr_encapsulation_scheme *)s->serialization;
  scheme->scheme = freertps_htons(FR_SCHEME_PL_CDR_LE);
  scheme->options = 0;
  s->serialized_len = 4;
}

void fr_parameter_list_append(
    struct fr_parameter_list *s,
    fr_parameter_id_t pid, void *value, uint16_t len)
{
  struct fr_parameter_list_item *item =
      (struct fr_parameter_list_item *)&s->serialization[s->serialized_len];
  item->pid = pid;
  item->len = len;
  if (value)
    memcpy(item->value, value, len);
  s->serialized_len += 4 + len;
}

