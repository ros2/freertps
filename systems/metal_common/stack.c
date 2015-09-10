#include "metal/stack.h"

volatile __attribute__((used,aligned(8),section(".stack"))) uint8_t g_stack[STACK_SIZE];
