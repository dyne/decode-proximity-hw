/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License (GPL) version 2. See the file LICENSE
 * for more details.
 *
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "dp3t-config.h"
#include "dp3t.h"
#include "random.h"
#include "nvmc.h"

static uint8_t SKT_0[SK_LEN] = {};
static int keystore_initialized = 0;
static int beacons_generated = 0;
static int day_ephid_table = -1;

static char broadcast_key[32] = BROADCAST_KEY;
static uint32_t broadcast_key_len = BROADCAST_KEY_LEN;

#define BEACONS_OBJ_SIZE (sizeof(beacons_t) + sizeof(beacon_t) * 16)
static uint8_t local_beacons_buffer[BEACONS_OBJ_SIZE];
static beacons_t *beacons = (beacons_t *)(local_beacons_buffer);

#define CONFIG_FLASH_ADDR ((512 - 4)  * (1024)) /* Last sector of flash considering a 512KB boundary */

const char dp3t_config_signature[4] = "DP3T";

struct __attribute__((packed)) dp3t_config_store {
    char signature[4]; /* spells "DP3T" if the store is in use */
    sk_t     key;      /* oldest key stored */
    uint32_t age;      /* age in days */
};

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
    static uint8_t zero16[16] = {};
    if (!keystore_initialized)
        return zero16;
    return beacons->ephids[idx];
}


static struct dp3t_config_store *load_config(void)
{
    struct dp3t_config_store *store = 
        (struct dp3t_config_store *)(CONFIG_FLASH_ADDR);
    if (strncmp(store->signature, dp3t_config_signature, 4) == 0) {
        printf("DP-3T configuration loaded from NVMC\n");
        print_sk(store->key);
        return store;
    }
    return NULL;
}

static void store_config(struct dp3t_config_store *sto)
{
    flash_erase(CONFIG_FLASH_ADDR, FLASH_PAGE_SIZE);
    flash_write(CONFIG_FLASH_ADDR, (uint8_t *)sto, sizeof(struct dp3t_config_store));
    printf("DP-3T configuration stored to NVMC\n");
}

uint8_t *dp3t_get_skt_0(void)
{
    if (keystore_initialized)
        return SKT_0;
    else return NULL;
}


static int rekey(void)
{
    struct dp3t_config_store *sto, newsto;
    sys_random(newsto.key, SK_LEN);
    memcpy(newsto.signature, dp3t_config_signature, 4);
    memcpy(newsto.key, newsto.key, SK_LEN);
    newsto.age = 0;
    store_config(&newsto);
    /* Sanity check for the new configuration */
    sto = load_config();
    if (!sto)
        return -1;
    return 0;
}


/* Initialize dp3t subsystem.
 * returns: 0 : Okay
 *         -1 : Problems with nvm store
 *         -2 : Could not generate beacons
 */
int dp3t_start(void)
{
    struct dp3t_config_store *sto;
    int ret;
    if (keystore_initialized)
        return 0;
    sto = load_config();
    if (!sto) {
        /* Create new random SK0, set age to 0 */
        rekey();
        sto = load_config();
        if (!sto)
            return -1;
    }
    memset(beacons, 0, BEACONS_OBJ_SIZE);
    ret = generate_beacons(beacons, EPOCHS, sto->key, sto->age, TTL, broadcast_key, broadcast_key_len); 
    if (ret != 0)
        return -2;
    keystore_initialized = 1;
    return 0;
}

int dp3t_restart(unsigned int day)
{
    struct dp3t_config_store *sto, newsto;
    int ret;
    if (keystore_initialized) {
        sto = load_config();
        if (sto != NULL) {
            /* don't update if the age is the same */
            if (sto->age == day)
                return 0;
            memcpy(&newsto, sto, sizeof(struct dp3t_config_store));
        } else {
            /* Generate new random key, don't set age */
            sys_random(SKT_0, SK_LEN);
            memcpy(newsto.key, SKT_0, SK_LEN);
        }
        keystore_initialized = 0;
    } else {
        sto = &newsto;
    }
    /* Store new configuration */
    memcpy(newsto.signature, dp3t_config_signature, 4);
    newsto.age = day;
    store_config(&newsto);
    /* Sanity check */
    if (!load_config())
        return -1;
    /* Generate beacons for the day */
    ret = generate_beacons(beacons, EPOCHS, sto->key, sto->age, TTL, broadcast_key, broadcast_key_len); 
    if (ret != 0)
        return -2;
    keystore_initialized = 1;
    return 0;
}


int dp3t_shellcmd_testvec(int argc, char **argv)
{
    uint8_t testkey[32] = { }; /* zeros */
    dp3t_generate_beacons(testkey, 0);
    return 0;
}

int dp3t_shellcmd_rekey(int argc, char **argv)
{
    return rekey();
}
