#include "metal/arm_trap.h"
#include <stdio.h>

void arm_trap_unhandled_vector(void)
{
  volatile int isr_number = (int)(__get_IPSR() & 0x1ff);
  printf("IT'S A TRAP!\r\n");
  if (isr_number < 16)
    printf("unhandled ARM vector %d !\r\n", isr_number);
  else
    printf("unhandled STM32 vector position %d (ARM ISR %d)!\r\n",
        isr_number - 16, isr_number);
  while (1) { } // spin here to allow jtag trap
}

