#ifndef STACK_H
#define STACK_H

#include <stdint.h>

#ifndef STACK_SIZE
#  define STACK_SIZE 0x4000
#endif

extern volatile uint8_t g_stack[STACK_SIZE];

#endif
