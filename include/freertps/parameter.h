#ifndef FREERTPS_PARAMETER_H
#define FREERTPS_PARAMETER_H

#include <stdint.h>

typedef uint16_t fr_parameter_id_t;
typedef struct fr_parameter_list_item
{
  fr_parameter_id_t pid;
  uint16_t len;
  uint8_t value[];
} __attribute__((packed)) fr_parameter_list_item_t;

typedef struct fr_parameter_list
{
  uint32_t serialized_len;
  uint8_t  serialization[];
} fr_parameter_list_t;

void fr_parameter_list_init(struct fr_parameter_list *s);
void fr_parameter_list_append(struct fr_parameter_list *s,
    fr_parameter_id_t pid, void *value, uint16_t len);

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


#endif

