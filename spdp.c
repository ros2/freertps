#include "freertps/freertps.h"
#include "freertps/spdp.h"
#include "freertps/udp.h"
//#include <arpa/inet.h>

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
  // todo: spin through this param list and save it
  fu_parameter_list_item_t *item = (fu_parameter_list_item_t *)data;
  while ((uint8_t *)item < submsg->contents + submsg->header.len)
  {
    const fu_parameterid_t pid = item->pid;
    if (pid == FU_PID_SENTINEL)
      break;
    FREERTPS_INFO("      spdp rx param 0x%x len %d\n", (unsigned)pid, item->len);
    const uint8_t *pval = item->value;
    // todo: do something with parameter value
    // now, advance to next item in list...
    item = (fu_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
  }
}

void freertps_spdp_init()
{
  FREERTPS_INFO("sdp init\n");
  frudp_entityid_t unknown = { .u = 0 };
  frudp_entityid_t writer  = { .s = { .key = { 0x00, 0x01, 0x00 }, .kind = 0xc2 } };
  frudp_subscribe(unknown, writer, frudp_spdp_rx);
}

void freertps_spdp_fini()
{
  FREERTPS_INFO("sdp fini\n");
}

