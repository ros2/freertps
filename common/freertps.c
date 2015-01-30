#include <stdlib.h>
#include "freertps/freertps.h"

void freertps_perish_if(bool b, const char *msg)
{
  if (b)
  {
    printf("%s\n", msg);
    exit(1);
  }
}

