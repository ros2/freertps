#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include "freertps/message.h"
#include "freertps/time.h"
#include <stdint.h>
#include <stdbool.h>
#include "freertps/guid.h"
#include "freertps/receiver.h"
#include "freertps/sequence_number.h"
#include "freertps/vendor_id.h"

typedef void (*fr_rx_data_cb_t)(fr_receiver_t *rcvr,
    const fr_submessage_t *submsg, const uint16_t scheme, const uint8_t *data);

/////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////

bool fr_init();
void fr_fini();


bool fr_add_mcast_rx(const uint32_t group,
                     const uint16_t port); //,
                               //const freertps_udp_rx_callback_t rx_cb);

// todo: elicit desired interface from the user in a sane way
bool fr_add_ucast_rx(const uint16_t port);

bool fr_listen(const uint32_t max_usec);

bool fr_rx(const uint32_t src_addr,
           const uint16_t src_port,
           const uint32_t dst_addr,
           const uint16_t dst_port,
           const uint8_t *rx_data,
           const uint16_t rx_len);

bool fr_tx(const uint32_t dst_addr,
           const uint16_t dst_port,
           const uint8_t *tx_data,
           const uint16_t tx_len);

uint16_t fr_spdp_port();

const char *fr_ip4_ntoa(const uint32_t addr);

struct fr_message *fr_init_msg(struct fr_message *buf);

#define FR_PLIST_ADVANCE(list_item) \
          do { \
            list_item = (fr_parameter_list_item_t *) \
                        (((uint8_t *)list_item) + 4 + list_item->len); \
          } while (0)

//extern const struct rtps_psm g_rtps_psm_udp;

#endif
