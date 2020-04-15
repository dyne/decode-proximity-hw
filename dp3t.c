/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License (GPL) version 2. See the file LICENSE
 * for more details.
 *
 */
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/hmac.h>

#include "nimble_scanner.h"
#include "net/bluetil/ad.h"
#include "nimble_scanlist.h"

#include "dp3t.h"

#define SK_LEN 32
#define SHA256_LEN 32
#define EPHID_LEN 16
#define EPOCH_LEN 15 // In minutes
#define EPOCHS_PER_DAY (((24 * 60) / EPOCH_LEN) + 1)
#define RETENTION_PERIOD (1) // In days
#define MAX_EPHIDS (RETENTION_PERIOD * EPOCHS_PER_DAY)

#define TRNG_BASE 0x4000D000
#define TRNG_TASKS_START (*(volatile uint32_t *)(TRNG_BASE + 0x000))
#define TRNG_TASKS_STOP  (*(volatile uint32_t *)(TRNG_BASE + 0x004))
#define TRNG_EV_VALRDY   (*(volatile uint32_t *)(TRNG_BASE + 0x100))
#define TRNG_VALUE       (*(volatile uint32_t *)(TRNG_BASE + 0x508))

static uint8_t SKT_0[SK_LEN] = {};
static int keystore_initialized = 0;
static int day_ephid_table = -1;

const uint8_t zeroes[SK_LEN] = {};
const uint8_t BROADCAST_KEY[] = "Broadcast key";
const uint32_t BROADCAST_KEY_LEN = 13;

void dp3t_random(uint8_t *buf, int len)
{
    uint8_t val[4];
    uint32_t *vu32 = (uint32_t *)val;
    int i = 0;
    
    /* Start TRNG */
    TRNG_TASKS_START = 1;

    for (;;) {
        /* Wait until value ready */
        while (TRNG_EV_VALRDY == 0)
            ;
        *vu32 = TRNG_VALUE;
        if (len >= 4)
            memcpy(buf + i, val, 4);
        else
            memcpy(buf + i, val, len);
        len -=4;
        i += 4;
        if (len <= 0)
            break;
    }
    TRNG_TASKS_STOP |= 1;
}



static uint8_t EPHIDS_LOCAL[MAX_EPHIDS][EPHID_LEN];

/* 
 * SKT0 is random at every power-on now
 * (should it be created once then stored in flash?)
 * TODO
 *
 */
uint8_t *dp3t_get_skt_0(void)
{
    if (!keystore_initialized) {
        dp3t_random(SKT_0, SK_LEN);
        keystore_initialized = 1;
    }
    return SKT_0;
}

/*
 *   This function creates the next key in the chain of SK_t's.
 *   It is called either for the local rotation or when we
 *   recover the different SK_ts from an infected person.
*/
void dp3t_get_skt_1(const uint8_t *skt_0, uint8_t *skt_1)
{
    int ret;
    wc_Sha256 sha;
    uint8_t digest[SHA256_LEN];
    ret = wc_InitSha256_ex(&sha, NULL, INVALID_DEVID);
    assert(ret > 0);
    ret = wc_Sha256Update(&sha, skt_0, SK_LEN);
    assert(ret > 0);
    wc_Sha256Final(&sha, skt_1);
    wc_Sha256Free(&sha);
}

void dp3t_create_ephids(void)
{
    unsigned buffer_size = EPHID_LEN * MAX_EPHIDS;
    Aes aes;
    Hmac hmac;
    uint8_t prf[SK_LEN], prg[SK_LEN];
    int i;
    /* PRF */
    wc_HmacInit(&hmac, NULL, INVALID_DEVID); 
    wc_HmacSetKey(&hmac, WC_SHA256, dp3t_get_skt_0(), SK_LEN);
    wc_HmacUpdate(&hmac, BROADCAST_KEY, BROADCAST_KEY_LEN);
    wc_HmacFinal(&hmac, prf);
    /* PRG */
    wc_AesInit(&aes, NULL, INVALID_DEVID);
    wc_AesSetKey(&aes, prf, 16, zeroes, 16);
    for (i = 0; i < EPOCHS_PER_DAY; i++) {
        wc_AesCtrEncrypt(&aes, EPHIDS_LOCAL[i], zeroes, EPHID_LEN); 
    }
}

uint8_t *dp3t_get_ephid(int epoch)
{
    return EPHIDS_LOCAL[epoch];
}

