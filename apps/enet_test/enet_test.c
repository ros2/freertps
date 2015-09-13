// minimal program to just sit there and send nonsense UDP while listening
// for pings. intended for sanity-checking and debugging low-level ethernet
#include <stdio.h>
#include <stdlib.h>
#include "metal/systime.h"
#include "metal/enet.h"
#include "freertps/udp.h"

#define TX_INTERVAL 1000000

int main(int argc, char **argv)
{
  enet_init();
  __enable_irq();
  printf("hello world\r\n");
  uint32_t last_tx_time = 0;
  int tx_count = 0;
  while (1)
  {
    enet_process_rx_ring();
    uint32_t t = systime_usecs();
    if (t - last_tx_time > TX_INTERVAL)
    {
      last_tx_time = t;
      printf("tx %d %d\r\n", tx_count++, (int)t);
      uint8_t payload[8] = {0};
      frudp_tx(0x6801a8c0, 5000, payload, sizeof(payload));
    }
  }
  return 0;
}
