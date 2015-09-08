#include "enet.h"
#include "samv71q21.h"
#include "net_config.h"
#include <stdio.h>

const uint8_t g_enet_mac[6] = ENET_MAC;

void enet_init()
{
  printf("enet_init()\r\n");
}


