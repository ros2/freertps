#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#ifndef FRUDP_MAX_MATCHED_READERS
#  define FRUDP_MAX_MATCHED_READERS 10
#endif

#include "freertps/id.h"
#include "freertps/udp.h"
#include "freertps/freertps.h"

// create userland UDP subscriptions. people should call the
// freertps_create_subscription() from userland code though, to be agnostic
// to the physical layer

// could be dangerous to hold onto string pointers... they need to be
// stored in the caller's constant memory. maybe revisit this at some point,
// with a #define switch somewhere to have it use more memory for string
// buffers, etc.

void frudp_create_subscription(const char *topic_name,
                               const char *type_name,
                               freertps_msg_cb_t msg_cb);

// this is the private subscribe function used internally... should be hidden
// eventually.
bool frudp_subscribe(const frudp_entity_id_t reader_id,
                     const frudp_entity_id_t writer_id,
                     const frudp_rx_data_cb_t data_cb,
                     const freertps_msg_cb_t msg_cb);

typedef struct
{
  frudp_guid_t writer_guid;
  frudp_entity_id_t reader_entity_id;
  frudp_sequence_number_t max_rx_sn;
  frudp_rx_data_cb_t data_cb;
  freertps_msg_cb_t msg_cb;
} frudp_matched_reader_t;

// not great to have these freely available. someday hide these.
extern frudp_matched_reader_t g_frudp_matched_readers[FRUDP_MAX_MATCHED_READERS];
extern unsigned g_frudp_num_matched_readers;

typedef struct
{
  const char *topic_name;
  const char *type_name;
  frudp_entity_id_t entity_id;
  freertps_msg_cb_t msg_cb;
} frudp_userland_subscription_request_t; // awful name

#define FRUDP_MAX_USERLAND_SUBS 10
extern frudp_userland_subscription_request_t g_frudp_userland_subs[FRUDP_MAX_USERLAND_SUBS];
extern uint32_t g_frudp_num_userland_subs;

#endif
