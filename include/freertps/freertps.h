#ifndef FREERTPS_H
#define FREERTPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

// NOTE: the prefix freertps_ is too long to type, so it will often
// be shortened to fr_


#include <stdint.h>

#include "freertps/udp.h"
#include "freertps/config.h"
#include "freertps/time.h"
#include "freertps/timer.h"
#include "freertps/ports.h"
#include "freertps/locator.h"
#include "freertps/discovery.h"
#include "freertps/bswap.h"
#include "freertps/system.h"
#include "freertps/writer.h"
#include "freertps/reader.h"
#include "freertps/message.h"

// maybe make this smarter someday
#define FREERTPS_INFO(...) \
  do { printf(__VA_ARGS__); } while (0)
#define FREERTPS_ERROR(...) \
  do { printf(__VA_ARGS__); } while (0)
#define FREERTPS_FATAL(...) \
  do { printf(__VA_ARGS__); } while (0)

#if 0
void freertps_create_sub(const char *topic_name,
                         const char *type_name,
                         freertps_msg_cb_t msg_cb);

// todo: come up with a better way of holding onto publishers that is
// agnostic to the physical layer
fr_pub_t *freertps_create_pub(const char *topic_name,
                              const char *type_name);

bool freertps_publish(fr_pub_t *pub,
                      const uint8_t *msg,
                      const uint32_t msg_len);
//void freertps_perish_if(bool b, const char *msg);
#endif

extern bool g_freertps_init_complete;

void freertps_start();

void freertps_init();
bool freertps_ok();
void freertps_fini();
void freertps_spin(const uint32_t microseconds);
void freertps_add_timer(uint32_t usec, freertps_timer_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif
