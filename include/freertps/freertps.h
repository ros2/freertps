#ifndef FREERTPS_H
#define FREERTPS_H

#include <stdio.h>
// make this smarter someday
#define FREERTPS_INFO(...) printf(__VA_ARGS__)

#include <stdbool.h>
void freertps_perish_if(bool b, const char *msg);

#endif
