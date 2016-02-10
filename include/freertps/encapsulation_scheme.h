#ifndef FREERTPS_ENCAPSULATION_SCHEME_H
#define FREERTPS_ENCAPSULATION_SCHEME_H

#include <stdint.h>

typedef struct fr_encapsulation_scheme
{
  uint16_t scheme;
  uint16_t options;
} __attribute__((packed)) fr_encapsulation_scheme_t;

#define FR_SCHEME_CDR_LE    0x0100
#define FR_SCHEME_PL_CDR_LE 0x0300

#endif

