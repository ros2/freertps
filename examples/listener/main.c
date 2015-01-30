#include <stdio.h>
#include "freertps/udp.h"

int main(int argc, char **argv)
{
  freertps_udp_init();
  freertps_udp_fini();
  return 0;
}

