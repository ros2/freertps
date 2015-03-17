#ifndef FREERTPS_SPDP_H
#define FREERTPS_SPDP_H

#include "freertps/udp.h"
#include <stdbool.h>

#define FRUDP_PID_PAD                           0x0000
#define FRUDP_PID_SENTINEL                      0x0001
#define FRUDP_PID_PROTOCOL_VERSION              0x0015
#define FRUDP_PID_VENDORID                      0x0016
#define FRUDP_PID_DEFAULT_UNICAST_LOCATOR       0x0031
#define FRUDP_PID_DEFAULT_MULTICAST_LOCATOR     0x0048
#define FRUDP_PID_METATRAFFIC_UNICAST_LOCATOR   0x0032
#define FRUDP_PID_METATRAFFIC_MULTICAST_LOCATOR 0x0033
#define FRUDP_PID_PARTICIPANT_LEASE_DURATION    0x0002
#define FRUDP_PID_PARTICIPANT_GUID              0x0050
#define FRUDP_PID_BUILTIN_ENDPOINT_SET          0x0058

#define FRUDP_BUILTIN_EP_PARTICIPANT_ANNOUNCER           0x00000001
#define FRUDP_BUILTIN_EP_PARTICIPANT_DETECTOR            0x00000002
#define FRUDP_BUILTIN_EP_PUBLICATION_ANNOUNCER           0x00000004
#define FRUDP_BUILTIN_EP_PUBLICATION_DETECTOR            0x00000008
#define FRUDP_BUILTIN_EP_SUBSCRIPTION_ANNOUNCER          0x00000010
#define FRUDP_BUILTIN_EP_SUBSCRIPTION_DETECTOR           0x00000020
#define FRUDP_BUILTIN_EP_PARTICIPANT_PROXY_ANNOUNCER     0x00000040
#define FRUDP_BUILTIN_EP_PARTICIPANT_PROXY_DETECTOR      0x00000080
#define FRUDP_BUILTIN_EP_PARTICIPANT_STATE_ANNOUNCER     0x00000100
#define FRUDP_BUILTIN_EP_PARTICIPANT_STATE_DETECTOR      0x00000200
#define FRUDP_BUILTIN_EP_PARTICIPANT_MESSAGE_DATA_WRITER 0x00000400
#define FRUDP_BUILTIN_EP_PARTICIPANT_MESSAGE_DATA_READER 0x00000800

typedef struct
{
  frudp_pver_t pver;
  frudp_vid_t vid;
  frudp_guid_prefix_t guid_prefix;
  bool expects_inline_qos;
  frudp_locator_t default_unicast_locator;
  frudp_locator_t default_multicast_locator;
  frudp_locator_t metatraffic_unicast_locator;
  frudp_locator_t metatraffic_multicast_locator;
  frudp_duration_t lease_duration;
  frudp_builtin_endpoint_set_t builtin_endpoints;
} frudp_participant_t;

void frudp_spdp_init();
void frudp_spdp_fini();
void frudp_spdp_tick();

#endif
