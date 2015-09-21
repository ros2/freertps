#ifndef FREERTPS_SPDP_H
#define FREERTPS_SPDP_H

#include "freertps/id.h"
extern const frudp_eid_t g_spdp_writer_id, g_spdp_reader_id;

void frudp_spdp_init();
void frudp_spdp_start();
void frudp_spdp_tick();
void frudp_spdp_fini();

void frudp_spdp_bcast();

#endif
