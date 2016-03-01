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

void fr_parameter_list_append_string(
    struct fr_parameter_list *s, fr_parameter_id_t pid, const char *str)
{
  struct fr_parameter_list_item *item =
      (struct fr_parameter_list_item *)&s->serialization[s->serialized_len];
  item->pid = pid;
  uint32_t *param_len = (uint32_t *)item->value;
  *param_len = strlen(str) + 1; // RTPS length includes the null char
  memcpy(((uint8_t *)item->value)+4, str, *param_len); // copy null char too
  // zero-pad up to 32-bit alignment
  uint32_t len_padded = (4 + *param_len + 3) & ~0x3; // round to 32-bit align
  for (uint32_t zero_pad_align = 4+*param_len; zero_pad_align < len_padded;
      zero_pad_align++)
    item->value[zero_pad_align] = 0;
  item->len = len_padded;
  s->serialized_len += 4 + len_padded;
}

