#ifndef FREERTPS_H
#define FREERTPS_H

#include <stdio.h>
// make this smarter someday
#define FREERTPS_INFO(...) \
  do { printf("freertps: "); printf(__VA_ARGS__); } while (0)

#include <stdbool.h>
void freertps_perish_if(bool b, const char *msg);

#endif
