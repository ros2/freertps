#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include "freertps/freertps.h"
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

// NOTE: the prefix freertps_udp_ is too long to type, so it will heretofore 
// be shortened to frudp_

// NOTE: everything is assumed to be little-endian. if we ever need to run on
// big-endian, we'll have to rethink some of this.

/////////////////////////////////////////////////////////////////////
// A FEW CONSTANTS FROM THE SPEC
/////////////////////////////////////////////////////////////////////

#define FRUDP_PORT_PB 7400
#define FRUDP_PORT_DG  250
#define FRUDP_PORT_PG    2
#define FRUDP_PORT_D0    0
#define FRUDP_PORT_D1   10
#define FRUDP_PORT_D2    1
#define FRUDP_PORT_D3   11

// to avoid conflicts / false-positives on our LAN
#define FRUDP_DOMAIN_ID  0

// default multicast group is 239.255.0.1
#define FRUDP_DEFAULT_MCAST_GROUP 0xefff0001

// for now let's pretend that our vendor ID is 11311 in hex
#define FREERTPS_VENDOR_ID 0x2C2F

/////////////////////////////////////////////////////////////////////
// TYPES 
/////////////////////////////////////////////////////////////////////

typedef struct 
{ 
  uint8_t major; 
  uint8_t minor; 
} frudp_pver_t; // protocol version

typedef uint16_t frudp_vid_t; // vendor ID
const char *frudp_vendor(const frudp_vid_t vid);

#define FRUDP_GUID_PREFIX_LEN 12
typedef struct
{
  uint8_t prefix[FRUDP_GUID_PREFIX_LEN];
} frudp_guid_prefix_t;

bool frudp_guid_prefix_identical(frudp_guid_prefix_t * const a,
                                 frudp_guid_prefix_t * const b);

typedef struct
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

#define FRUDP_FLAGS_LITTLE_ENDIAN 0x1
#define FRUDP_FLAGS_INLINE_QOS    0x2
#define FRUDP_FLAGS_DATA_PRESENT  0x4

#define FRUDP_SUBMSG_ID_INFO_TS 0x09
#define FRUDP_SUBMSG_ID_DATA    0x15

typedef struct
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

typedef union
{
  struct
  {
    uint8_t key[3];
    uint8_t kind;
  } s;
  uint32_t u;
} __attribute__((packed)) frudp_entity_id_t;

extern const frudp_entity_id_t g_frudp_entity_id_unknown;

#define FRUDP_ENTITYID_BUILTIN_SDP_PARTICIPANT_WRITER 0x000100c2

typedef struct
{
  frudp_guid_prefix_t guid_prefix;
  frudp_entity_id_t entity_id;
} __attribute__((packed)) frudp_guid_t;

typedef struct
{
  frudp_pver_t       src_pver;
  frudp_vid_t        src_vid;
  frudp_guid_prefix_t src_guid_prefix;
  frudp_guid_prefix_t dst_guid_prefix;
  bool            have_timestamp;
  fr_time_t       timestamp;
} frudp_receiver_state_t;


typedef struct
{
  int32_t high;
  uint32_t low;
} frudp_sequence_number_t;

typedef struct
{
  frudp_sequence_number_t bitmap_base;
  uint32_t num_bits;
  uint32_t bitmap[];
} frudp_sequence_number_set_t;

typedef struct
{
  uint16_t extraflags;
  uint16_t octets_to_inline_qos;
  frudp_entity_id_t reader_id;
  frudp_entity_id_t writer_id;
  frudp_sequence_number_t writer_sn;
} __attribute__((packed)) frudp_submsg_contents_data_t;

typedef struct
{
  frudp_entity_id_t reader_id;
  frudp_entity_id_t writer_id;
  frudp_sequence_number_t first_sn;
  frudp_sequence_number_t last_sn;
  uint32_t count;
} __attribute__((packed)) frudp_submsg_heartbeat_t;

typedef struct
{
  frudp_entity_id_t reader_id;
  frudp_entity_id_t writer_id;
  frudp_sequence_number_set_t reader_sn_state_t;
  uint32_t count;
} __attribute__((packed)) frudp_submsg_acknack_t;

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

#define FRUDP_ENCAPSULATION_SCHEME_PL_CDR_LE 0x0003

typedef void (*frudp_rx_cb_t)(frudp_receiver_state_t *rcvr,
                              const frudp_submsg_t *submsg,
                              const uint16_t scheme,
                              const uint8_t *data);

typedef struct
{
  frudp_entity_id_t reader_id;
  frudp_entity_id_t writer_id;
  frudp_rx_cb_t cb;
} frudp_subscription_t;

#ifndef FRUDP_MAX_SUBSCRIPTIONS
#  define FRUDP_MAX_SUBSCRIPTIONS 10
#endif

typedef struct
{
  int32_t kind;
  uint32_t port;
  union
  {
    uint8_t raw[16];
    struct
    {
      uint8_t zeros[12];
      uint32_t addr;
    } udp4;
  } addr; 
} __attribute__((packed)) frudp_locator_t;

#define FRUDP_LOCATOR_KIND_INVALID -1
#define FRUDP_LOCATOR_KIND_RESERVED 0
#define FRUDP_LOCATOR_KIND_UDPV4    1
#define FRUDP_LOCATOR_KIND_UDPV6    2

typedef struct
{
  int32_t sec;
  uint32_t nanosec;
} frudp_duration_t;

typedef uint32_t frudp_builtin_endpoint_set_t;

typedef struct
{
  frudp_guid_prefix_t guid_prefix;
  int participant_id;
  uint32_t unicast_addr;
} frudp_config_t;
extern frudp_config_t g_frudp_config;

typedef struct
{
  uint32_t len;
  uint8_t data[];
} frudp_rtps_string_t;

/////////////////////////////////////////////////////////////////////
// FUNCTIONS 
/////////////////////////////////////////////////////////////////////

bool frudp_init();
void frudp_fini();

bool frudp_generic_init();
bool frudp_init_participant_id();

bool frudp_add_mcast_rx(const in_addr_t group, 
                        const uint16_t port); //,
                               //const freertps_udp_rx_callback_t rx_cb);

// todo: elicit desired interface from the user in a sane way
bool frudp_add_ucast_rx(const uint16_t port);

bool frudp_listen(const uint32_t max_usec);

bool frudp_rx(const in_addr_t src_addr,
              const in_port_t src_port,
              const in_addr_t dst_addr,
              const in_port_t dst_port,
              const uint8_t *rx_data,
              const uint16_t rx_len);

bool frudp_subscribe(const frudp_entity_id_t reader_id,
                     const frudp_entity_id_t writer_id,
                     const frudp_rx_cb_t cb);

bool frudp_tx(const in_addr_t dst_addr,
              const in_port_t dst_port,
              const uint8_t *tx_data,
              const uint16_t tx_len);

uint16_t frudp_ucast_builtin_port();
uint16_t frudp_mcast_builtin_port();
uint16_t frudp_ucast_user_port();
uint16_t frudp_mcast_user_port();
uint16_t frudp_spdp_port();

const char *frudp_ip4_ntoa(const uint32_t addr);

bool frudp_parse_string(char *buf, uint32_t buf_len, frudp_rtps_string_t *s);

frudp_msg_t *frudp_init_msg(uint8_t *buf);

#endif

