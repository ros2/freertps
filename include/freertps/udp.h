#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include "freertps/time.h"
#include <stdint.h>
#include <stdbool.h>
#include "freertps/id.h"
//#include "freertps/psm.h"

/////////////////////////////////////////////////////////////////////
// TYPES
/////////////////////////////////////////////////////////////////////

typedef struct
{
  uint8_t major;
  uint8_t minor;
} frudp_pver_t; // protocol version

typedef struct frudp_header
{
  uint32_t magic_word; // RTPS in ASCII
  frudp_pver_t pver; // protocol version
  frudp_vid_t  vid;  // vendor ID
  frudp_guid_prefix_t guid_prefix;
} frudp_header_t;

typedef struct
{
  frudp_header_t header;
  uint8_t submsgs[];
} frudp_msg_t;

#define FRUDP_FLAGS_LITTLE_ENDIAN      0x01
#define FRUDP_FLAGS_INLINE_QOS         0x02
#define FRUDP_FLAGS_DATA_PRESENT       0x04

#define FRUDP_FLAGS_ACKNACK_FINAL      0x02

#define FRUDP_SUBMSG_ID_ACKNACK        0x06
#define FRUDP_SUBMSG_ID_HEARTBEAT      0x07
#define FRUDP_SUBMSG_ID_INFO_TS        0x09
#define FRUDP_SUBMSG_ID_INFO_DEST      0x0e
#define FRUPG_SUBMSG_ID_HEARTBEAT_FRAG 0x13
#define FRUDP_SUBMSG_ID_DATA           0x15
#define FRUDP_SUBMSG_ID_DATA_FRAG      0x16

typedef struct frudp_submsg_header
{
  uint8_t id;
  uint8_t flags;
  uint16_t len;
} frudp_submsg_header_t;

typedef struct
{
  frudp_submsg_header_t header;
  uint8_t contents[];
} frudp_submsg_t;

typedef struct
{
  frudp_pver_t        src_pver;
  frudp_vid_t         src_vid;
  frudp_guid_prefix_t src_guid_prefix;
  frudp_guid_prefix_t dst_guid_prefix;
  bool                have_timestamp;
  fr_time_t           timestamp;
} frudp_receiver_state_t;

typedef struct
{
  int32_t high;
  uint32_t low;
} frudp_sn_t; // sequence number
extern const frudp_sn_t g_frudp_sn_unknown;

typedef struct
{
  frudp_sn_t bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap[];
} frudp_sn_set_t;

typedef struct
{
  frudp_sn_t bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap;
} frudp_sn_set_32bits_t;

typedef struct
{
  frudp_submsg_header_t header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  frudp_eid_t reader_id;
  frudp_eid_t writer_id;
  frudp_sn_t writer_sn;
  uint8_t data[];
} __attribute__((packed)) frudp_submsg_data_t;

typedef struct frudp_submsg_data_frag
{
  struct frudp_submsg_header header;
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  frudp_eid_t reader_id;
  frudp_eid_t writer_id;
  frudp_sn_t writer_sn;
  uint32_t fragment_starting_number;
  uint16_t fragments_in_submessage;
  uint16_t fragment_size;
  uint32_t sample_size;
  uint8_t data[];
} __attribute__((packed)) frudp_submsg_data_frag_t;

typedef struct
{
  frudp_submsg_header_t header;
  frudp_eid_t reader_id;
  frudp_eid_t writer_id;
  frudp_sn_t first_sn;
  frudp_sn_t last_sn;
  uint32_t count;
} __attribute__((packed)) frudp_submsg_heartbeat_t;

typedef struct
{
  frudp_submsg_header_t header;
  frudp_eid_t reader_id;
  frudp_eid_t writer_id;
  frudp_sn_t gap_start;
  frudp_sn_set_t gap_end;
} __attribute__((packed)) frudp_submsg_gap_t;

typedef struct
{
  frudp_eid_t reader_id;
  frudp_eid_t writer_id;
  frudp_sn_set_t reader_sn_state;
  // the "count" field that goes here is impossible to declare in legal C
} __attribute__((packed)) frudp_submsg_acknack_t;

typedef struct
{
  frudp_guid_prefix_t guid_prefix;
} __attribute__((packed)) frudp_submsg_info_dest_t;

typedef uint16_t frudp_parameterid_t;
typedef struct
{
  frudp_parameterid_t pid;
  uint16_t len;
  uint8_t value[];
} __attribute__((packed)) frudp_parameter_list_item_t;

typedef struct
{
  uint16_t scheme;
  uint16_t options;
} __attribute__((packed)) frudp_encapsulation_scheme_t;

#define FRUDP_SCHEME_CDR_LE    0x0001
#define FRUDP_SCHEME_PL_CDR_LE 0x0003

typedef void (*frudp_rx_data_cb_t)(frudp_receiver_state_t *rcvr,
                                   const frudp_submsg_t *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data);

typedef struct
{
  int32_t sec;
  uint32_t nanosec;
} frudp_duration_t;

typedef uint32_t frudp_builtin_endpoint_set_t;

typedef struct
{
  uint32_t len;
  uint8_t data[];
} frudp_rtps_string_t;

/////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////

bool frudp_init(void);
void frudp_fini(void);

bool frudp_generic_init(void);
bool frudp_init_participant_id(void);

bool frudp_add_mcast_rx(const uint32_t group,
                        const uint16_t port); //,
                               //const freertps_udp_rx_callback_t rx_cb);

// todo: elicit desired interface from the user in a sane way
bool frudp_add_ucast_rx(const uint16_t port);

bool frudp_listen(const uint32_t max_usec);

bool frudp_rx(const uint32_t src_addr,
              const uint16_t src_port,
              const uint32_t dst_addr,
              const uint16_t dst_port,
              const uint8_t *rx_data,
              const uint16_t rx_len);

bool frudp_tx(const uint32_t dst_addr,
              const uint16_t dst_port,
              const uint8_t *tx_data,
              const uint16_t tx_len);

uint16_t frudp_ucast_builtin_port(void);
uint16_t frudp_mcast_builtin_port(void);
uint16_t frudp_ucast_user_port(void);
uint16_t frudp_mcast_user_port(void);
uint16_t frudp_spdp_port(void);

const char *frudp_ip4_ntoa(const uint32_t addr);

bool frudp_parse_string(char *buf, uint32_t buf_len, frudp_rtps_string_t *s);

frudp_msg_t *frudp_init_msg(frudp_msg_t *buf);

#define FRUDP_PLIST_ADVANCE(list_item) \
          do { \
            list_item = (frudp_parameter_list_item_t *) \
                        (((uint8_t *)list_item) + 4 + list_item->len); \
          } while (0)

extern const struct rtps_psm g_rtps_psm_udp;

#endif
