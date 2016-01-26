#ifndef FREERTPS_VENDOR_ID_H
#define FREERTPS_VENDOR_ID_H

// for now let's pretend that our vendor ID is 11311 in hex
#define FREERTPS_VENDOR_ID 0x2C2F
typedef uint16_t fr_vendor_id_t;

const char *fr_vendor_id_string(const fr_vendor_id_t vid);

#endif
