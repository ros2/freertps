#include "freertps/disco.h"
#include "freertps/spdp.h"
#include "freertps/sedp.h"

uint8_t g_frudp_disco_tx_buf[FRUDP_DISCO_TX_BUFLEN];
uint16_t g_frudp_disco_tx_buf_wpos;

frudp_part_t g_frudp_disco_parts[FRUDP_DISCO_MAX_PARTS];
int g_frudp_disco_num_parts = 0;

////////////////////////////////////////////////////////////////

void frudp_disco_init(void)
{
  FREERTPS_INFO("discovery init\r\n");
  frudp_spdp_init();
  frudp_sedp_init();
}

void frudp_disco_start(void)
{
  FREERTPS_INFO("discovery start\r\n");
  frudp_spdp_start();
  frudp_sedp_start();
}

void frudp_disco_fini(void)
{
  FREERTPS_INFO("discovery fini\r\n");
  frudp_spdp_fini();
  frudp_sedp_fini();
}

void frudp_disco_tick(void)
{
  frudp_spdp_tick();
  frudp_sedp_tick();
}
