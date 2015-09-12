// minimal program to print the system time to the console
#include <stdio.h>
#include <stdlib.h>
#include "metal/systime.h"

int main(int argc, char **argv)
{
  srand(0);
  while (1)
  {
    for (volatile int i = 0; i < 200000; i++);
    uint32_t rand_offset = rand() % 10000;
    for (volatile int i = 0; i < rand_offset; i++);
    //float f = 1.0e-6 * (float)systime_usecs();
    //printf("%.6f\r\n", (double)f);
    systime_usecs();
    //printf("%4d\r\n", (int)t);
    /*
    double f = (double)t * 10.0f;
    printf("%10d    %f\r\n", (int)t, f);
    */
  }
  return 0;
}

