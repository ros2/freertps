#ifndef FR_PARTICIPANT_H
#define FR_PARTICIPANT_H

#include <stdbool.h>
#include "freertps/container.h"
#include "freertps/locator.h"
#include "freertps/protocol_version.h"
#include "freertps/udp.h"
#include "freertps/reader.h"
#include "freertps/vendor_id.h"
#include "freertps/writer.h"

typedef struct fr_participant
{
  struct fr_guid_prefix guid_prefix;
  struct fr_protocol_version protocol_version;
  fr_vendor_id_t vendor_id;
  uint32_t domain_id, participant_id;
  struct fr_container *default_unicast_locators;
  struct fr_container *default_multicast_locators;
  struct fr_container *matched_participants;
  struct fr_container *writers;
  struct fr_container *readers;
} fr_participant_t;

fr_participant_t *fr_participant_find(const fr_guid_prefix_t *guid_prefix);
bool fr_participant_init();
void fr_participant_fini();
bool fr_participant_add_writer(struct fr_writer *writer);
bool fr_participant_add_reader(struct fr_reader *reader);

extern struct fr_participant g_fr_participant;

uint16_t fr_participant_ucast_builtin_port();
uint16_t fr_participant_mcast_builtin_port();
uint16_t fr_participant_ucast_user_port();
uint16_t fr_participant_mcast_user_port();

bool fr_participant_add_default_locators();

void fr_participant_print_locators();

#endif
