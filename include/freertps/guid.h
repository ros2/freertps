#ifndef FR_GUID_H
#define FR_GUID_H

#include <stdint.h>
#include <stdbool.h>

typedef union fr_entity_id
{
  struct
  {
    uint8_t key[3];
    uint8_t kind;
  } s;
  uint32_t u;
} __attribute__((packed)) fr_entity_id_t;

#define FR_ENTITY_ID_PARTICIPANT 0xc1010000
#define FR_ENTITY_KIND_USER_WRITER_WITH_KEY 0x02
#define FR_ENTITY_KIND_USER_WRITER_NO_KEY   0x03
#define FR_ENTITY_KIND_USER_READER_NO_KEY   0x04
#define FR_ENTITY_KIND_USER_READER_WITH_KEY 0x07

extern const fr_entity_id_t g_fr_entity_id_unknown;

#define FR_GUID_PREFIX_LEN 12
typedef struct fr_guid_prefix
{
  uint8_t prefix[FR_GUID_PREFIX_LEN];
} fr_guid_prefix_t;

typedef struct fr_guid
{
  struct fr_guid_prefix prefix;
  union fr_entity_id entity_id;
} __attribute__((packed)) fr_guid_t;
extern const fr_guid_t g_fr_guid_unknown;

bool fr_guid_prefix_identical(struct fr_guid_prefix * const a,
                              struct fr_guid_prefix * const b);

bool fr_guid_identical(const fr_guid_t * const a,
                       const fr_guid_t * const b);

void fr_guid_stuff(struct fr_guid *guid,
                   const struct fr_guid_prefix *prefix,
                   const fr_entity_id_t *id);

void fr_guid_print_prefix(const fr_guid_prefix_t *guid_prefix);
void fr_guid_print(const fr_guid_t *guid);

fr_entity_id_t fr_create_user_id(const uint8_t entity_kind);

void fr_guid_set_zero(struct fr_guid *guid);

#endif
