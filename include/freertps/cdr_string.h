#ifndef FR_CDR_STRING_H
#define FR_CDR_STRING_H

#include <stdint.h>
#include <stdbool.h>

typedef struct fr_cdr_string
{
  uint32_t len;
  uint8_t data[];
} fr_cdr_string_t;

bool fr_cdr_string_parse(char *buf, uint32_t buf_len, struct fr_cdr_string *s);

#endif
