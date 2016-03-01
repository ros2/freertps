#ifndef FREERTPS_CONFIG_H
#define FREERTPS_CONFIG_H

#include <stdint.h>
#include "freertps/guid.h"


// default multicast group is 239.255.0.1
//#define FR_DEFAULT_MCAST_GROUP 0xefff0001
#define FR_DEFAULT_MCAST_GROUP 0x0100ffef

#define VERBOSE_MSG_RX
//#define VERBOSE_HEARTBEAT
//#define VERBOSE_DATA
//#define VERBOSE_ACKNACK
//#define VERBOSE_GAP

//#define VERBOSE_TX_ACKNACK
//#define SEDP_VERBOSE

#endif
