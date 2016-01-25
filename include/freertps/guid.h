#ifndef FR_GUID_H
#define FR_GUID_H

#include <stdint.h>
#include <stdbool.h>

typedef union
{
  struct
  {
    uint8_t key[3];
    uint8_t kind;
  } s;
  uint32_t u;
} __attribute__((packed)) fr_eid_t; // entity ID

#define FR_ENTITY_KIND_USER_WRITER_WITH_KEY 0x02
#define FR_ENTITY_KIND_USER_WRITER_NO_KEY   0x03
#define FR_ENTITY_KIND_USER_READER_NO_KEY   0x04
#define FR_ENTITY_KIND_USER_READER_WITH_KEY 0x07

extern const fr_eid_t g_fr_eid_unknown;

#define FR_GUID_PREFIX_LEN 12
typedef struct
{
  uint8_t prefix[FR_GUID_PREFIX_LEN];
} fr_guid_prefix_t;

typedef struct fr_guid
{
  fr_guid_prefix_t prefix;
  fr_eid_t eid;
} __attribute__((packed)) fr_guid_t;
extern const fr_guid_t g_fr_guid_unknown;

bool fr_guid_prefix_identical(fr_guid_prefix_t * const a,
                              fr_guid_prefix_t * const b);

bool fr_guid_identical(const fr_guid_t * const a,
                       const fr_guid_t * const b);

void fr_stuff_guid(fr_guid_t *guid,
                   const fr_guid_prefix_t *prefix,
                   const fr_eid_t *id);

/////////////////////////////////////////////////////////////////////////
// VENDOR ID STUFF
// for now let's pretend that our vendor ID is 11311 in hex
#define FREERTPS_VENDOR_ID 0x2C2F
typedef uint16_t fr_vid_t;
const char *fr_vendor(const fr_vid_t vid);

void fr_guid_print_prefix(const fr_guid_prefix_t *guid_prefix);
void fr_guid_print(const fr_guid_t *guid);

fr_eid_t fr_create_user_id(const uint8_t entity_kind);

#endif
