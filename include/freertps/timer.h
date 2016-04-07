#ifndef FREERTPS_TIMER_H
#define FREERTPS_TIMER_H

#include <stdint.h>

typedef void (*freertps_timer_cb_t)(void);
void freertps_timer_set_freq(uint32_t freq, freertps_timer_cb_t cb);

#endif
