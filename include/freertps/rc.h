#ifndef FREERTPS_RC_H
#define FREERTPS_RC_H

// many freertps functions have return codes. here is the list of them.
typedef uint32_t fr_rc_t;

#define FR_RC_OK              0
#define FR_RC_NOT_IMPLEMENTED 1
#define FR_RC_BAD_ARGUMENT    2
#define FR_RC_CONTAINER_FULL  3
#define FR_RC_NETWORK_ERROR   4

// I would have used the traditional "AHHHHHHHHHHHH" but it's hard to remember
// the correct number of H's, and different situations sometimes could
// require a different H count. "OH NOES" is similar and somewhat more precise
#define FR_RC_OH_NOES        99

#endif

