/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the Affero GNU
 * General Public License (AGPL) version 3. See the file LICENSE for
 * more details.
 *
 */
#ifndef NVMC_H
#define NVMC_H
#define FLASH_PAGE_SIZE (4096)
int flash_write(uint32_t address, const uint8_t *data, int len);
int flash_erase(uint32_t address, int len);
#endif
