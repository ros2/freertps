#ifndef SYSTIME_H
#define SYSTIME_H

#include <stdint.h>

void systime_init(void);

// can this be inlined somehow and defined in a chip-specific library?
// i kind of doubt it, but i dunno.
uint32_t systime_usecs(void);

#endif

