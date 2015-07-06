#include "freertps/id.h"
#include <stdio.h>
#include <string.h>

unsigned g_frudp_next_user_entity_id = 1;

const frudp_guid_t g_frudp_guid_unknown = { .guid_prefix = { .prefix = {0} },
                                            .entity_id = { .u = 0 } };

const char *frudp_vendor(const frudp_vid_t vid)
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

bool frudp_guid_prefix_identical(frudp_guid_prefix_t * const a,
                                 frudp_guid_prefix_t * const b)
{
  for (int i = 0; i < FRUDP_GUID_PREFIX_LEN; i++)
    if (a->prefix[i] != b->prefix[i])
      return false;
  return true;
}

bool frudp_guid_identical(const frudp_guid_t * const a,
                          const frudp_guid_t * const b)
{
  if (a->entity_id.u != b->entity_id.u)
    return false;
  for (int i = 0; i < FRUDP_GUID_PREFIX_LEN; i++)
    if (a->guid_prefix.prefix[i] != b->guid_prefix.prefix[i])
      return false;
  return true;
}

void frudp_print_guid_prefix(frudp_guid_prefix_t *p)
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

void frudp_stuff_guid(frudp_guid_t *guid,
                      const frudp_guid_prefix_t *prefix,
                      const frudp_entity_id_t *id)
{
  memcpy(&guid->guid_prefix, prefix, sizeof(frudp_guid_prefix_t));
  guid->entity_id = *id;
}

void frudp_print_guid(frudp_guid_t *guid)
{
  frudp_print_guid_prefix(&guid->guid_prefix);
  printf("%08x", (unsigned)guid->entity_id.u);
}
