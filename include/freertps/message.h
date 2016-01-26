#ifndef FREERTPS_MESSAGE_H
#define FREERTPS_MESSAGE_H

#include "freertps/guid.h"
#include "freertps/protocol_version.h"
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

typedef struct fr_header
{
  uint32_t magic_word; // RTPS in ASCII
  struct fr_protocol_version protocol_version; // protocol version
  fr_vendor_id_t vendor_id;  // vendor ID
  struct fr_guid_prefix guid_prefix;
} fr_header_t;

typedef struct fr_message
{
  struct fr_header header;
  uint8_t submessages[];
} fr_message_t;

typedef struct fr_submessage_header
{
  uint8_t id;
  uint8_t flags;
  uint16_t len;
} fr_submessage_header_t;

typedef struct fr_submessage
{
  struct fr_submessage_header header;
  uint8_t contents[];
} fr_submessage_t;

typedef struct fr_data_submessage
{
  struct fr_submessage_header header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  struct fr_sequence_number writer_sn;
  uint8_t data[];
} __attribute__((packed)) fr_data_submessage_t;

typedef struct fr_data_frag_submessage
{
  struct fr_submessage_header header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
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
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  struct fr_sequence_number first_sn;
  struct fr_sequence_number last_sn;
  uint32_t count;
} __attribute__((packed)) fr_heartbeat_submessage_t;

typedef struct fr_gap_submessage
{
  struct fr_submessage_header header;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  struct fr_sequence_number gap_start;
  struct fr_sequence_number_set gap_end;
} __attribute__((packed)) fr_gap_submessage_t;

typedef struct fr_acknack_submessage
{
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  struct fr_sequence_number_set reader_sn_state;
  // the "count" field that goes here is impossible to declare in legal C
} __attribute__((packed)) fr_acknack_submessage_t;

typedef struct fr_info_dest_submessage
{
  struct fr_guid_prefix guid_prefix;
} __attribute__((packed)) fr_info_dest_submessage_t;

typedef uint16_t fr_parameter_id_t;
typedef struct fr_parameter_list_item
{
  fr_parameter_id_t pid;
  uint16_t len;
  uint8_t value[];
} __attribute__((packed)) fr_parameter_list_item_t;

typedef struct fr_encapsulation_scheme
{
  uint16_t scheme;
  uint16_t options;
} __attribute__((packed)) fr_encapsulation_scheme_t;

#define FR_SCHEME_CDR_LE    0x0001
#define FR_SCHEME_PL_CDR_LE 0x0003

#endif
