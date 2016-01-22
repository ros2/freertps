#include "freertps/id.h"
#include <stdio.h>
#include <string.h>
#include "freertps/bswap.h"

static unsigned g_fr_next_user_eid = 1;

const fr_guid_t g_fr_guid_unknown = { .prefix = { .prefix = {0} },
                                      .eid = { .u = 0 } };

const char *fr_vendor(const fr_vid_t vid)
{
  switch (vid)
  {
    case 0x0101: return "RTI Connext";
    case 0x0102: return "PrismTech OpenSplice";
    case 0x0103: return "OCI OpenDDS";
    case 0x0104: return "MilSoft";
    case 0x0105: return "Gallium InterCOM";
    case 0x0106: return "TwinOaks CoreDX";
    case 0x0107: return "Lakota Technical Systems";
    case 0x0108: return "ICOUP Consulting";
    case 0x0109: return "ETRI";
    case 0x010a: return "RTI Connext Micro";
    case 0x010b: return "PrismTech Vortex Cafe";
    case 0x010c: return "PrismTech Vortex Gateway";
    case 0x010d: return "PrismTech Vortex Lite";
    case 0x010e: return "Technicolor Qeo";
    case 0x010f: return "eProsima";
    case 0x0120: return "PrismTech Vortex Cloud";
    case FREERTPS_VENDOR_ID: return "freertps";
    default:     return "unknown";
  }
}

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
  if (a->eid.u != b->eid.u)
    return false;
  for (int i = 0; i < FR_GUID_PREFIX_LEN; i++)
    if (a->prefix.prefix[i] != b->prefix.prefix[i])
      return false;
  return true;
}

void fr_print_guid_prefix(const fr_guid_prefix_t *p)
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
                   const fr_eid_t *id)
{
  memcpy(&guid->prefix, prefix, sizeof(fr_guid_prefix_t));
  guid->eid = *id;
}

void fr_print_guid(const fr_guid_t *guid)
{
  fr_print_guid_prefix(&guid->prefix);
  printf(":%08x", (unsigned)freertps_htonl(guid->eid.u));
}

fr_eid_t fr_create_user_id(const uint8_t entity_kind)
{
  printf("fr_create_user_id()\r\n");
  fr_eid_t eid;
  eid.s.kind = entity_kind; // entity kind must be set by caller of this functionmust be overwritten by FR_ENTITY_KIND_USER_READER_NO_KEY; // has key? dunno
  eid.s.key[0] = 0;
  eid.s.key[1] = 0; // todo: >8 bit ID's
  eid.s.key[2] = g_fr_next_user_eid++;
  return eid;
}
