#include "freertps/discovery.h"
#include "freertps/freertps.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"

uint8_t g_fr_discovery_tx_buf[FR_DISCOVERY_TX_BUFLEN];
uint16_t g_fr_discovery_tx_buf_wpos;

fr_participant_t g_fr_discovery_participants[FR_DISCOVERY_MAX_PARTICIPANTS];
int g_fr_discovery_num_participants = 0;

////////////////////////////////////////////////////////////////

void fr_discovery_init()
{
  FREERTPS_INFO("discovery init\r\n");
  fr_spdp_init();
  fr_sedp_init();
}

void fr_discovery_start()
{
  FREERTPS_INFO("discovery start\r\n");
  fr_spdp_start();
  fr_sedp_start();
}

void fr_discovery_fini()
{
  FREERTPS_INFO("discovery fini\r\n");
  fr_spdp_fini();
  fr_sedp_fini();
}

void fr_discovery_tick()
{
  fr_spdp_tick();
  fr_sedp_tick();
}
