#include "freertps/guid.h"
#include <stdio.h>
#include <string.h>
#include "freertps/bswap.h"

////////////////////////////////////////////////////////////////////////////
// global constants
const fr_guid_t g_fr_guid_unknown = { .prefix = { .prefix = {0} },
                                      .entity_id = { .u = 0 } };
const fr_entity_id_t g_fr_entity_id_unknown = { .u = 0 };
///////////////////////////////////////////////////////////////////////////
static unsigned g_fr_next_user_entity_id = 1;
///////////////////////////////////////////////////////////////////////////

bool fr_guid_prefix_identical(fr_guid_prefix_t * const a,
                              fr_guid_prefix_t * const b)
{
  for (int i = 0; i < FR_GUID_PREFIX_LEN; i++)
    if (a->prefix[i] != b->prefix[i])
      return false;
  return true;
}

bool fr_guid_identical(const fr_guid_t * const a,
                       const fr_guid_t * const b)
{
  if (a->entity_id.u != b->entity_id.u)
    return false;
  for (int i = 0; i < FR_GUID_PREFIX_LEN; i++)
    if (a->prefix.prefix[i] != b->prefix.prefix[i])
      return false;
  return true;
}

void fr_guid_print_prefix(const fr_guid_prefix_t *p)
{
  printf("%02x%02x%02x%02x:%02x%02x%02x%02x:%02x%02x%02x%02x",
         (unsigned)p->prefix[0],
         (unsigned)p->prefix[1],
         (unsigned)p->prefix[2],
         (unsigned)p->prefix[3],
         (unsigned)p->prefix[4],
         (unsigned)p->prefix[5],
         (unsigned)p->prefix[6],
         (unsigned)p->prefix[7],
         (unsigned)p->prefix[8],
         (unsigned)p->prefix[9],
         (unsigned)p->prefix[10],
         (unsigned)p->prefix[11]);
}

void fr_stuff_guid(fr_guid_t *guid,
                   const fr_guid_prefix_t *prefix,
                   const fr_entity_id_t *id)
{
  memcpy(&guid->prefix, prefix, sizeof(fr_guid_prefix_t));
  guid->entity_id = *id;
}

void fr_guid_print(const fr_guid_t *guid)
{
  fr_guid_print_prefix(&guid->prefix);
  printf(":%08x", (unsigned)freertps_htonl(guid->entity_id.u));
}

fr_entity_id_t fr_create_user_id(const uint8_t entity_kind)
{
  printf("fr_create_user_id()\r\n");
  fr_entity_id_t entity_id;
  entity_id.s.kind = entity_kind; // entity kind must be set by caller of this functionmust be overwritten by FR_ENTITY_KIND_USER_READER_NO_KEY; // has key? dunno
  entity_id.s.key[0] = 0;
  entity_id.s.key[1] = 0; // todo: >8 bit ID's
  entity_id.s.key[2] = g_fr_next_user_entity_id++;
  return entity_id;
}

void fr_guid_set_zero(struct fr_guid *guid)
{
  for (int i = 0; i < FR_GUID_PREFIX_LEN; i++)
    guid->prefix.prefix[i] = 0;
  guid->entity_id.u = 0;
}
