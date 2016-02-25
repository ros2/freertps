#ifndef FREERTPS_DISCOVERY_H
#define FREERTPS_DISCOVERY_H

// this has a bunch of discovery-related debris

#include <stdint.h>
#include "freertps/participant.h"
#include "freertps/config.h"

#define FR_BUILTIN_EP_PARTICIPANT_ANNOUNCER           0x00000001
#define FR_BUILTIN_EP_PARTICIPANT_DETECTOR            0x00000002
#define FR_BUILTIN_EP_PUBLICATION_ANNOUNCER           0x00000004
#define FR_BUILTIN_EP_PUBLICATION_DETECTOR            0x00000008
#define FR_BUILTIN_EP_SUBSCRIPTION_ANNOUNCER          0x00000010
#define FR_BUILTIN_EP_SUBSCRIPTION_DETECTOR           0x00000020
#define FR_BUILTIN_EP_PARTICIPANT_PROXY_ANNOUNCER     0x00000040
#define FR_BUILTIN_EP_PARTICIPANT_PROXY_DETECTOR      0x00000080
#define FR_BUILTIN_EP_PARTICIPANT_STATE_ANNOUNCER     0x00000100
#define FR_BUILTIN_EP_PARTICIPANT_STATE_DETECTOR      0x00000200
#define FR_BUILTIN_EP_PARTICIPANT_MESSAGE_DATA_WRITER 0x00000400
#define FR_BUILTIN_EP_PARTICIPANT_MESSAGE_DATA_READER 0x00000800

void fr_discovery_init();
void fr_discovery_fini();

//#define FR_DISCOVERY_TX_BUFLEN 1536
//extern uint8_t g_fr_discovery_tx_buf[FR_DISCOVERY_TX_BUFLEN];
//extern uint16_t g_fr_discovery_tx_buf_wpos;

//extern fr_participant_t g_fr_discovery_participants[FR_DISCOVERY_MAX_PARTICIPANTS];
//extern int g_fr_discovery_num_participants;

#endif
