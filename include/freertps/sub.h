#ifndef FREERTPS_SUBSCRIPTION_H
#define FREERTPS_SUBSCRIPTION_H

#include "freertps/id.h"
#include "freertps/udp.h"
#include "freertps/freertps.h"
#include "freertps/config.h"

// create userland UDP subscriptions. people should call the
// freertps_create_subscription() from userland code though, to be agnostic
// to the physical layer

// could be dangerous to hold onto string pointers... they need to be
// stored in the caller's constant memory. maybe revisit this at some point,
// with a #define switch somewhere to have it use more memory for string
// buffers, etc.

void fr_add_user_sub(const char *topic_name,
                     const char *type_name,
                     freertps_msg_cb_t msg_cb);

// this is the private subscribe function used internally... should be hidden
// eventually.
/*
bool fr_subscribe(const fr_entity_id_t reader_id,
                  const fr_entity_id_t writer_id,
                  const fr_rx_data_cb_t data_cb,
                  const freertps_msg_cb_t msg_cb);
*/

typedef struct
{
  const char *topic_name;
  const char *type_name;
  fr_eid_t reader_eid;
  fr_rx_data_cb_t data_cb;
  freertps_msg_cb_t msg_cb;
  bool reliable;
} fr_sub_t;

void fr_add_sub(const fr_sub_t *s);

extern fr_sub_t g_fr_subs[FR_MAX_SUBS];
extern uint32_t g_fr_num_subs;

typedef struct
{
  bool reliable;
  fr_guid_t writer_guid;
  fr_eid_t reader_eid;
  fr_sequence_number_t max_rx_sn;
  fr_rx_data_cb_t data_cb;
  freertps_msg_cb_t msg_cb;
} fr_reader_t;

// not great to have these freely available. someday hide these.
extern fr_reader_t g_fr_readers[FR_MAX_READERS];
extern uint32_t g_fr_num_readers;

void fr_add_reader(const fr_reader_t *reader);

void fr_print_readers();

#endif
