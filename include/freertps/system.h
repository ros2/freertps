#ifndef FREERTPS_POSIX_SYSTEM_H
#define FREERTPS_POSIX_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

void fr_system_init();
bool fr_system_ok();
void fr_system_fini();
int  fr_system_listen_at_most(uint32_t microseconds);

#endif
