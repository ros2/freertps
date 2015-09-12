// minimal program to print the system time to the console
#include <stdio.h>
#include <stdlib.h>
#include "metal/systime.h"

int main(int argc, char **argv)
{
  while (1)
  {
    uint32_t waste_time = 200000 + rand() % 10000;
    for (volatile int i = 0; i < waste_time; i++);
    float f = 1.0e-6 * (float)systime_usecs();
    printf("%.6f\r\n", (double)f);
  }
  return 0;
}
