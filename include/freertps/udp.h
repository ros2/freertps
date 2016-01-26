#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include "freertps/time.h"
#include <stdint.h>
#include <stdbool.h>
#include "freertps/guid.h"
#include "freertps/protocol_version.h"
#include "freertps/receiver.h"
#include "freertps/seq_num.h"
#include "freertps/vendor_id.h"

/////////////////////////////////////////////////////////////////////
// TYPES
/////////////////////////////////////////////////////////////////////

typedef struct fr_header
{
  uint32_t magic_word; // RTPS in ASCII
  struct fr_protocol_version protocol_version; // protocol version
  fr_vendor_id_t vendor_id;  // vendor ID
  struct fr_guid_prefix guid_prefix;
} fr_header_t;

typedef struct
{
  fr_header_t header;
  uint8_t submsgs[];
} fr_msg_t;

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

typedef struct fr_submsg_header
{
  uint8_t id;
  uint8_t flags;
  uint16_t len;
} fr_submsg_header_t;

typedef struct
{
  fr_submsg_header_t header;
  uint8_t contents[];
} fr_submsg_t;

typedef struct
{
  fr_submsg_header_t header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  fr_seq_num_t writer_sn;
  uint8_t data[];
} __attribute__((packed)) fr_submsg_data_t;

typedef struct fr_submsg_data_frag
{
  fr_submsg_header_t header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  fr_seq_num_t writer_sn;
  uint32_t fragment_starting_number;
  uint16_t fragments_in_submessage;
  uint16_t fragment_size;
  uint32_t sample_size;
  uint8_t data[];
} __attribute__((packed)) fr_submsg_data_frag_t;

typedef struct
{
  fr_submsg_header_t header;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  fr_seq_num_t first_sn;
  fr_seq_num_t last_sn;
  uint32_t count;
} __attribute__((packed)) fr_submsg_heartbeat_t;

typedef struct
{
  fr_submsg_header_t header;
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  fr_seq_num_t gap_start;
  fr_seq_num_set_t gap_end;
} __attribute__((packed)) fr_submsg_gap_t;

typedef struct
{
  fr_entity_id_t reader_id;
  fr_entity_id_t writer_id;
  fr_seq_num_set_t reader_sn_state;
  // the "count" field that goes here is impossible to declare in legal C
} __attribute__((packed)) fr_submsg_acknack_t;

typedef struct
{
  fr_guid_prefix_t guid_prefix;
} __attribute__((packed)) fr_submsg_info_dest_t;

typedef uint16_t fr_parameterid_t;
typedef struct
{
  fr_parameterid_t pid;
  uint16_t len;
  uint8_t value[];
} __attribute__((packed)) fr_parameter_list_item_t;

typedef struct
{
  uint16_t scheme;
  uint16_t options;
} __attribute__((packed)) fr_encapsulation_scheme_t;

#define FR_SCHEME_CDR_LE    0x0001
#define FR_SCHEME_PL_CDR_LE 0x0003

typedef void (*fr_rx_data_cb_t)(fr_receiver_t *rcvr,
                                const fr_submsg_t *submsg,
                                const uint16_t scheme,
                                const uint8_t *data);

typedef struct
{
  uint32_t len;
  uint8_t data[];
} fr_rtps_string_t;

/////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////

bool fr_init();
void fr_fini();

bool fr_generic_init();
bool fr_init_participant_id();

bool fr_add_mcast_rx(const uint32_t group,
                     const uint16_t port); //,
                               //const freertps_udp_rx_callback_t rx_cb);

// todo: elicit desired interface from the user in a sane way
bool fr_add_ucast_rx(const uint16_t port);

bool fr_listen(const uint32_t max_usec);

bool fr_rx(const uint32_t src_addr,
           const uint16_t src_port,
           const uint32_t dst_addr,
           const uint16_t dst_port,
           const uint8_t *rx_data,
           const uint16_t rx_len);

bool fr_tx(const uint32_t dst_addr,
           const uint16_t dst_port,
           const uint8_t *tx_data,
           const uint16_t tx_len);

uint16_t fr_ucast_builtin_port();
uint16_t fr_mcast_builtin_port();
uint16_t fr_ucast_user_port();
uint16_t fr_mcast_user_port();
uint16_t fr_spdp_port();

const char *fr_ip4_ntoa(const uint32_t addr);

bool fr_parse_string(char *buf, uint32_t buf_len, fr_rtps_string_t *s);

fr_msg_t *fr_init_msg(fr_msg_t *buf);

#define FR_PLIST_ADVANCE(list_item) \
          do { \
            list_item = (fr_parameter_list_item_t *) \
                        (((uint8_t *)list_item) + 4 + list_item->len); \
          } while (0)

//extern const struct rtps_psm g_rtps_psm_udp;

#endif
