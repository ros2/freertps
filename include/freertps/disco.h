#ifndef FREERTPS_DISCO_H
#define FREERTPS_DISCO_H

// this has a bunch of discovery-related debris

#include <stdint.h>
#include "freertps/part.h"
#include "freertps/config.h"

#define FR_PID_PAD                           0x0000
#define FR_PID_SENTINEL                      0x0001
#define FR_PID_PARTICIPANT_LEASE_DURATION    0x0002
#define FR_PID_TOPIC_NAME                    0x0005
#define FR_PID_TYPE_NAME                     0x0007
#define FR_PID_RELIABILITY                   0x001a
#define FR_PID_PROTOCOL_VERSION              0x0015
#define FR_PID_VENDOR_ID                     0x0016
#define FR_PID_RELIABILITY                   0x001a
#define FR_PID_LIVELINESS                    0x001b
#define FR_PID_DURABILITY                    0x001d
#define FR_PID_PRESENTATION                  0x0021
#define FR_PID_PARTITION                     0x0029
#define FR_PID_DEFAULT_UNICAST_LOCATOR       0x0031
#define FR_PID_METATRAFFIC_UNICAST_LOCATOR   0x0032
#define FR_PID_METATRAFFIC_MULTICAST_LOCATOR 0x0033
#define FR_PID_HISTORY                       0x0040
#define FR_PID_DEFAULT_MULTICAST_LOCATOR     0x0048
#define FR_PID_TRANSPORT_PRIORITY            0x0049
#define FR_PID_PARTICIPANT_GUID              0x0050
#define FR_PID_BUILTIN_ENDPOINT_SET          0x0058
#define FR_PID_PROPERTY_LIST                 0x0059
#define FR_PID_ENDPOINT_GUID                 0x005a
#define FR_PID_KEY_HASH                      0x0070

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

void fr_disco_init();
void fr_disco_fini();

void fr_disco_start(); /// must be called to kick off discovery
void fr_disco_tick();  /// must be called periodically to broadcast SPDP

#define FR_DISCO_TX_BUFLEN 1536
extern uint8_t g_fr_disco_tx_buf[FR_DISCO_TX_BUFLEN];
extern uint16_t g_fr_disco_tx_buf_wpos;

extern fr_part_t g_fr_disco_parts[FR_DISCO_MAX_PARTS];
extern int g_fr_disco_num_parts;

#endif
