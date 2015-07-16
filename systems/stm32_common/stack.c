#include "stack.h"

__attribute__((aligned(8),section(".stack"))) uint8_t g_stack[STACK_SIZE];
