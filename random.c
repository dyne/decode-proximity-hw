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




#define TRNG_BASE 0x4000D000
#define TRNG_TASKS_START (*(volatile uint32_t *)(TRNG_BASE + 0x000))
#define TRNG_TASKS_STOP  (*(volatile uint32_t *)(TRNG_BASE + 0x004))
#define TRNG_EV_VALRDY   (*(volatile uint32_t *)(TRNG_BASE + 0x100))
#define TRNG_VALUE       (*(volatile uint32_t *)(TRNG_BASE + 0x508))

void sys_random(uint8_t *buf, int len)
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



