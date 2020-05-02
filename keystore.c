#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "dp3t-config.h"
#include "dp3t.h"
#include "random.h"


static uint8_t SKT_0[SK_LEN] = {};
static int keystore_initialized = 0;
static int beacons_generated = 0;
static int day_ephid_table = -1;

static char broadcast_key[32] = BROADCAST_KEY;
static uint32_t broadcast_key_len = BROADCAST_KEY_LEN;

#define BEACONS_OBJ_SIZE (sizeof(beacons_t) + sizeof(beacon_t) * 16)
static uint8_t local_beacons_buffer[BEACONS_OBJ_SIZE];
static beacons_t *beacons = (beacons_t *)(local_beacons_buffer);


static void print_hex(const uint8_t *x, int len)
{
    int i;
    for(i = 0; i < len; i++) {
        printf("%02x",x[i]);
    }
    printf("\n");
}

static void print_ephid(const uint8_t *x)
{
    print_hex(x, EPHID_LEN);
}

static void print_sk(const uint8_t *x)
{
    print_hex(x, SK_LEN);
}


void dp3t_print_ephids(void)
{
    int i;
    for (i = 0; i < EPOCHS; i++) {
        printf("[ %03d ] ", i);
        print_ephid(beacons->ephids[i]);
    }
}

uint8_t *dp3t_get_skt_0(void)
{
    if (!keystore_initialized) {
        sys_random(SKT_0, SK_LEN);
        keystore_initialized = 1;
    }
    return SKT_0;
}

beacons_t *dp3t_generate_beacons(sk_t key, int day)
{
    int ret;
    memset(beacons, 0, BEACONS_OBJ_SIZE);
    ret = generate_beacons(beacons, EPOCHS, key, day, TTL, broadcast_key, broadcast_key_len); 
    if (ret != 0)
        return NULL;
    dp3t_print_ephids();
    return beacons;
}

uint8_t *dp3t_get_ephid(uint8_t idx)
{
    return beacons->ephids[idx];
}


