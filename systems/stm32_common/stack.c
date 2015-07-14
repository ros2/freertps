#include "stack.h"

__attribute__((aligned(8),section(".stack"))) uint8_t s_stack_arr[STACK_SIZE];
const uint8_t *g_stack_top = &s_stack_arr[STACK_SIZE-8];
