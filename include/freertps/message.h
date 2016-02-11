#ifndef FREERTPS_MESSAGE_H
#define FREERTPS_MESSAGE_H

#include "freertps/encapsulation_scheme.h"
#include "freertps/guid.h"
#include "freertps/parameter.h"
#include "freertps/protocol_version.h"
#include "freertps/receiver.h"
#include "freertps/sequence_number.h"
#include "freertps/vendor_id.h"

#define FR_FLAGS_LITTLE_ENDIAN      0x01
#define FR_FLAGS_INLINE_QOS         0x02
#define FR_FLAGS_DATA_PRESENT       0x04

#define FR_FLAGS_ACKNACK_FINAL      0x02

#define FR_SUBMSG_ID_ACKNACK        0x06
#define FR_SUBMSG_ID_HEARTBEAT      0x07
#define FR_SUBMSG_ID_INFO_TS        0x09
#define FR_SUBMSG_ID_INFO_DEST      0x0e
#define FR_SUBMSG_ID_HEARTBEAT_FRAG 0x13
#define FR_SUBMSG_ID_DATA           0x15
#define FR_SUBMSG_ID_DATA_FRAG      0x16

struct fr_header
{
  uint32_t magic_word; // RTPS in ASCII
  struct fr_protocol_version protocol_version; // protocol version
  fr_vendor_id_t vendor_id;  // vendor ID
  struct fr_guid_prefix guid_prefix;
} __attribute__((packed));

struct fr_message
{
  struct fr_header header;
  uint8_t submessages[];
} __attribute__((packed));

struct fr_submessage_header
{
  uint8_t id;
  uint8_t flags;
  uint16_t len;
} __attribute__((packed));

struct fr_submessage
{
  struct fr_submessage_header header;
  uint8_t contents[];
} __attribute__((packed));

struct fr_submessage_data
{
  struct fr_submessage_header header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  union fr_entity_id reader_id;
  union fr_entity_id writer_id;
  struct fr_sequence_number writer_sn;
  uint8_t data[];
} __attribute__((packed));

typedef struct fr_data_frag_submessage
{
  struct fr_submessage_header header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  union fr_entity_id reader_id;
  union fr_entity_id writer_id;
  struct fr_sequence_number writer_sn;
  uint32_t fragment_starting_number;
  uint16_t fragments_in_submessage;
  uint16_t fragment_size;
  uint32_t sample_size;
  uint8_t data[];
} __attribute__((packed)) fr_data_frag_submessage_t;

typedef struct fr_heartbeat_submessage
{
  struct fr_submessage_header header;
  union fr_entity_id reader_id;
  union fr_entity_id writer_id;
  struct fr_sequence_number first_sn;
  struct fr_sequence_number last_sn;
  uint32_t count;
} __attribute__((packed)) fr_heartbeat_submessage_t;

typedef struct fr_gap_submessage
{
  struct fr_submessage_header header;
  union fr_entity_id reader_id;
  union fr_entity_id writer_id;
  struct fr_sequence_number gap_start;
  struct fr_sequence_number_set gap_end;
} __attribute__((packed)) fr_gap_submessage_t;

struct fr_submessage_acknack
{
  union fr_entity_id reader_id;
  union fr_entity_id writer_id;
  struct fr_sequence_number_set reader_sn_state;
  // the "count" field that goes here is impossible to declare in legal C
} __attribute__((packed));

typedef struct fr_info_dest_submessage
{
  struct fr_guid_prefix guid_prefix;
} __attribute__((packed)) fr_info_dest_submessage_t;

//////////////////////////////////////////////////////////////////////////
struct fr_message *fr_message_init(struct fr_message *buf);
void fr_message_rx(struct fr_receiver *receiver,
    const struct fr_submessage *submsg);

typedef void (*fr_message_rx_data_cb_t)(struct fr_receiver *rcvr,
    const struct fr_submessage *submsg, const uint16_t scheme,
    const uint8_t *data);

#endif

