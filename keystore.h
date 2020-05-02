#ifndef KEYSTORE_H
#define KEYSTORE_H
#include "dp3t.h"
#include "dp3t-config.h"
#include <stdint.h>

uint8_t *dp3t_get_skt_0(void);
beacons_t *dp3t_generate_beacons(sk_t key, int day);
uint8_t *dp3t_get_ephid(uint8_t idx);
#endif
