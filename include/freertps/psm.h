#ifndef FREERTPS_PSM_H
#define FREERTPS_PSM_H

#include "freertps/pub.h"

typedef bool (*rtps_psm_init_func_t)(void);
typedef void (*rtps_psm_disco_func_t)(void);
typedef frudp_pub_t *(*rtps_psm_create_pub_func_t)(
    const char *topic_name, const char *type_name);
typedef void (*rtps_psm_create_sub_func_t)(
    const char *topic_name, const char *type_name, freertps_msg_cb_t msg_cb);
typedef void (*rtps_psm_pub_func_t)(
    void *pub, const uint8_t *msg, const uint32_t msg_len);

struct rtps_psm
{
  rtps_psm_init_func_t       init;
  rtps_psm_disco_func_t      disco;
  rtps_psm_create_pub_func_t create_pub;
  rtps_psm_create_sub_func_t create_sub;
  rtps_psm_pub_func_t        pub;
};

extern struct rtps_psm g_rtps_psm;

#endif
