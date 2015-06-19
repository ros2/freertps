#include "freertps/discovery.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"

uint8_t g_frudp_discovery_tx_buf[FRUDP_DISCOVERY_TX_BUFLEN];
uint16_t g_frudp_discovery_tx_buf_wpos;

frudp_participant_t g_frudp_discovery_participants
                                   [FRUDP_DISCOVERY_MAX_PARTICIPANTS];
int g_frudp_discovery_num_participants = 0;

////////////////////////////////////////////////////////////////

void frudp_discovery_init()
{
  FREERTPS_INFO("discovery init\n");
  frudp_spdp_init();
  frudp_sedp_init();
}

void frudp_discovery_fini()
{
  FREERTPS_INFO("discovery fini\n");
  frudp_spdp_fini();
  frudp_sedp_fini();
}

void frudp_discovery_tick()
{
  frudp_spdp_tick();
  frudp_sedp_tick();
}
