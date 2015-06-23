#ifndef FREERTPS_H
#define FREERTPS_H

#include <stdio.h>

// NOTE: the prefix freertps_udp_ is too long to type, so it will often
// be shortened to frudp_

#include "freertps/config.h"
#include "freertps/time.h"
#include "freertps/ports.h"
#include "freertps/locator.h"

// maybe make this smarter someday
#define FREERTPS_INFO(...) \
  do { printf("freertps INFO : "); printf(__VA_ARGS__); } while (0)
#define FREERTPS_ERROR(...) \
  do { printf("freertps ERROR: "); printf(__VA_ARGS__); } while (0)
#define FREERTPS_FATAL(...) \
  do { printf("freertps FATAL: "); printf(__VA_ARGS__); } while (0)

//void freertps_perish_if(bool b, const char *msg);
#endif
