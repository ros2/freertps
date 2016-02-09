#ifndef FREERTPS_PROTOCOL_VERSION_H
#define FREERTPS_PROTOCOL_VERSION_H

typedef struct fr_protocol_version
{
  uint8_t major;
  uint8_t minor;
} fr_protocol_version_t;

#define FR_PROTOCOL_VERSION_MAJOR 2
#define FR_PROTOCOL_VERSION_MINOR 1

#endif
