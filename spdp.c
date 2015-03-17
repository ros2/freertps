#include "freertps/freertps.h"
#include "freertps/spdp.h"
#include "freertps/udp.h"
#include <string.h>

#define FRUDP_MAX_PARTICIPANTS 10
static frudp_participant_t g_frudp_spdp_participants[FRUDP_MAX_PARTICIPANTS];
static int frudp_spdp_num_participants = 0;

static frudp_participant_t g_frudp_spdp_rx_participant; // just for rx buffer

//////////////////////////////////////////////////////////////////////////

static void frudp_spdp_rx(fu_receiver_state_t *rcvr,
                          const fu_submsg_t *submsg,
                          const uint16_t scheme,
                          const uint8_t *data)
{
  FREERTPS_INFO("    spdp_rx\n");
  if (scheme != FRUDP_DATA_ENCAP_SCHEME_PL_CDR_LE)
  {
    FREERTPS_ERROR("expected spdp data to be PL_CDR_LE. bailing...\n");
    return;
  }
  frudp_participant_t *part = &g_frudp_spdp_rx_participant;
  // todo: spin through this param list and save it
  fu_parameter_list_item_t *item = (fu_parameter_list_item_t *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    const fu_parameterid_t pid = item->pid;
    if (pid == FRUDP_PID_SENTINEL)
      break;
    const uint8_t *pval = item->value;
    /*
    FREERTPS_INFO("      unhandled spdp rx param 0x%x len %d\n", 
                  (unsigned)pid, item->len);
    */
    if (pid == FRUDP_PID_PROTOCOL_VERSION)
    {
      FREERTPS_INFO("      spdp proto version 0x%04x\n", *((uint16_t *)(pval)));
      part->pver = *((frudp_pver_t *)(pval)); // todo: what about alignment?
    }
    else if (pid == FRUDP_PID_VENDORID)
    {
      part->vid = htons(*((frudp_vid_t *)pval));
      FREERTPS_INFO("      spdp vendorid 0x%04x = %s\n", part->vid, frudp_vendor(part->vid));
    }
    else if (pid == FRUDP_PID_DEFAULT_UNICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->default_unicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
        FREERTPS_INFO("      spdp unicast locator udpv4: %d.%d.%d.%d:%d\n",
                      loc->address[12],
                      loc->address[13],
                      loc->address[14],
                      loc->address[15],
                      loc->port);
      }
    }
    else if (pid == FRUDP_PID_DEFAULT_MULTICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->default_multicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
        FREERTPS_INFO("      spdp multicast locator udpv4: %d.%d.%d.%d:%d\n",
                      loc->address[12],
                      loc->address[13],
                      loc->address[14],
                      loc->address[15],
                      loc->port);
      }
      else
        FREERTPS_INFO("        spdp unknown mcast locator kind: %d\n", loc->kind);
    }
    else if (pid == FRUDP_PID_METATRAFFIC_UNICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->metatraffic_unicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
        FREERTPS_INFO("      spdp metatraffic unicast locator udpv4: %d.%d.%d.%d:%d\n",
                      loc->address[12], loc->address[13],
                      loc->address[14], loc->address[15],
                      loc->port);
      }
      else
        FREERTPS_INFO("        spdp unknown metatraffic mcast locator kind: %d\n", loc->kind);
    }
    else if (pid == FRUDP_PID_METATRAFFIC_MULTICAST_LOCATOR)
    {
      frudp_locator_t *loc = (frudp_locator_t *)pval;
      part->metatraffic_multicast_locator = *loc; // todo: worry about alignment
      if (loc->kind == FRUDP_LOCATOR_KIND_UDPV4)
      {
        FREERTPS_INFO("      spdp metatraffic multicast locator udpv4: %d.%d.%d.%d:%d\n",
                      loc->address[12], loc->address[13],
                      loc->address[14], loc->address[15],
                      loc->port);
      }
      else
        FREERTPS_INFO("        spdp unknown metatraffic mcast locator kind: %d\n", loc->kind);
    }
    else if (pid == FRUDP_PID_PARTICIPANT_LEASE_DURATION)
    {
      frudp_duration_t *dur = (frudp_duration_t *)pval;
      part->lease_duration = *dur;
      FREERTPS_INFO("      spdp lease duration: %d.%09d\n", 
                    dur->sec, dur->nanosec);
    }
    else if (pid == FRUDP_PID_PARTICIPANT_GUID)
    {
      frudp_guid_t *guid = (frudp_guid_t *)pval;
      memcpy(&part->guid_prefix, &guid->guid_prefix, FRUDP_GUIDPREFIX_LEN);
      uint8_t *p = guid->guid_prefix;
      FREERTPS_INFO("      guid 0x%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x\n",
                    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
                    p[8], p[9], p[10], p[11]);
    }
    else if (pid == FRUDP_PID_BUILTIN_ENDPOINT_SET)
    {
      part->builtin_endpoints = *((frudp_builtin_endpoint_set_t *)pval);
      FREERTPS_INFO("      builtin endpoints: 0x%08x\n", 
                    part->builtin_endpoints);
    }
    else
      FREERTPS_INFO("      unhandled spdp rx param 0x%x len %d\n", 
                    (unsigned)pid, item->len);

    // todo: do something with parameter value
    // now, advance to next item in list...
    item = (fu_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
}

void frudp_spdp_init()
{
  FREERTPS_INFO("sdp init\n");
  frudp_entityid_t unknown = { .u = 0 };
  frudp_entityid_t writer  = { .s = { .key = { 0x00, 0x01, 0x00 }, .kind = 0xc2 } };
  frudp_subscribe(unknown, writer, frudp_spdp_rx);
}

void frudp_spdp_fini()
{
  FREERTPS_INFO("sdp fini\n");
}

void frudp_spdp_tick()
{
  FREERTPS_INFO("spdp tick\n");
}

