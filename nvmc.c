/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the Affero GNU
 * General Public License (AGPL) version 3. See the file LICENSE for
 * more details.
 *
 */
#include <stdint.h>
#include "nvmc.h"

/* Flash write/erase control */
#define NVMC_BASE (0x4001E000)
#define NVMC_CONFIG *((volatile uint32_t *)(NVMC_BASE + 0x504))
#define NVMC_ERASEPAGE *((volatile uint32_t *)(NVMC_BASE + 0x508))
#define NVMC_READY *((volatile uint32_t *)(NVMC_BASE + 0x400))
#define NVMC_CONFIG_REN 0
#define NVMC_CONFIG_WEN 1
#define NVMC_CONFIG_EEN 2



static void flash_wait_complete(void)
{
    while (NVMC_READY == 0)
        ;
}

int flash_write(uint32_t address, const uint8_t *data, int len)
{
    int i = 0;
    uint32_t *src, *dst;

    while (i < len) {
        if ((len - i > 3) && ((((address + i) & 0x03) == 0)  && ((((uint32_t)data) + i) & 0x03) == 0)) {
            src = (uint32_t *)data;
            dst = (uint32_t *)address;
            NVMC_CONFIG = NVMC_CONFIG_WEN;
            flash_wait_complete();
            dst[i >> 2] = src[i >> 2];
            flash_wait_complete();
            i+=4;
        } else {
            uint32_t val;
            uint8_t *vbytes = (uint8_t *)(&val);
            int off = (address + i) - (((address + i) >> 2) << 2);
            dst = (uint32_t *)(address - off);
            val = dst[i >> 2];
            vbytes[off] = data[i];
            NVMC_CONFIG = NVMC_CONFIG_WEN;
            flash_wait_complete();
            dst[i >> 2] = val;
            flash_wait_complete();
            i++;
        }
    }
    return 0;
}


int flash_erase(uint32_t address, int len)
{
    uint32_t end = address + len - 1;
    uint32_t p;
    for (p = address; p <= end; p += FLASH_PAGE_SIZE) {
        NVMC_CONFIG = NVMC_CONFIG_EEN;
        flash_wait_complete();
        NVMC_ERASEPAGE = p;
        flash_wait_complete();
    }
    return 0;
}

