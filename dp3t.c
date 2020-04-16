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

const uint8_t BROADCAST_KEY[32] = "Broadcast key";
const uint32_t BROADCAST_KEY_LEN = 13;

void dp3t_random(uint8_t *buf, int len)
{
    uint8_t val;
    int i = 0;

    /* Clear VALRDY */
    TRNG_EV_VALRDY = 0;
    /* Start TRNG */
    TRNG_TASKS_START = 1;
    for (i = 0; i < len; i++) {
        /* Wait until value ready */
        while (TRNG_EV_VALRDY == 0)
            ;
        buf[i] = (uint8_t)(TRNG_VALUE & 0x000000FF);
        TRNG_EV_VALRDY = 0;
    }
    TRNG_TASKS_STOP |= 1;
}



static uint8_t EPHIDS_LOCAL[EPOCHS_PER_DAY][EPHID_LEN];

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
    ret = wc_InitSha256(&sha);
    assert(ret == 0);
    ret = wc_Sha256Update(&sha, skt_0, SK_LEN);
    assert(ret == 0);
    wc_Sha256Final(&sha, skt_1);
    wc_Sha256Free(&sha);
}

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
    for (i = 0; i < EPOCHS_PER_DAY; i++) {
        printf("[ %03d ] ", i);
        print_ephid(EPHIDS_LOCAL[i]);
    }
}

void dp3t_create_ephids(const uint8_t *skt_0)
{
    unsigned buffer_size = EPHID_LEN * MAX_EPHIDS;
    Aes aes;
    Hmac hmac;
    uint8_t prf[SK_LEN], sk1[SK_LEN];
    int i;
    uint8_t zeroes[EPHID_LEN];
    memset(zeroes, 0, SK_LEN);
    printf("SK0: ");
    print_sk(skt_0);

    /* PRF */
    wc_HmacInit(&hmac, NULL, INVALID_DEVID); 
    wc_HmacSetKey(&hmac, WC_SHA256, skt_0, SK_LEN);
    wc_HmacSetKey(&hmac, WC_SHA256, zeroes, 32);

    printf("Broadcast key: ");
    print_hex(BROADCAST_KEY, BROADCAST_KEY_LEN);
    printf("Zeroes: ");
    print_hex(zeroes, 32);
    wc_HmacUpdate(&hmac, BROADCAST_KEY, BROADCAST_KEY_LEN);
    wc_HmacFinal(&hmac, prf);
    printf("  PRF: ");
    print_sk(prf);

    /* Rotation test */
    dp3t_get_skt_1(skt_0, sk1);
    printf("  SK Derivation: ");
    print_sk(sk1);

    /* PRG */
    wc_AesInit(&aes, NULL, INVALID_DEVID);
    wc_AesSetKeyDirect(&aes, prf, 32, zeroes, AES_ENCRYPTION);
    for(i = 0; i < EPOCHS_PER_DAY; i++)
        wc_AesCtrEncrypt(&aes, EPHIDS_LOCAL[i], zeroes, 16); 
    dp3t_print_ephids();
    wc_HmacFree(&hmac);
    wc_AesFree(&aes);
}


uint8_t *dp3t_get_ephid(int epoch)
{
    return EPHIDS_LOCAL[epoch];
}

