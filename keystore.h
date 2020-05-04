/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the Affero GNU
 * General Public License (AGPL) version 3. See the file LICENSE for
 * more details.
 *
 */
#ifndef KEYSTORE_H
#define KEYSTORE_H
#include "dp3t.h"
#include "dp3t-config.h"
#include <stdint.h>

int dp3t_start(void);
int dp3t_restart(unsigned int day);
uint8_t *dp3t_get_ephid(uint8_t idx);

uint8_t *dp3t_get_skt_0(void);
beacons_t *dp3t_generate_beacons(sk_t key, int day);


int dp3t_shellcmd_testvec(int argc, char **argv);
int dp3t_shellcmd_rekey(int argc, char **argv);
#endif
