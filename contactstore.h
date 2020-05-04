/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the Affero GNU
 * General Public License (AGPL) version 3. See the file LICENSE for
 * more details.
 *
 */
#ifndef CONTACTSTORE_H
#define CONTACTSTORE_H
#include <stdint.h>
int cstore_add( uint8_t day, uint8_t epoch, uint8_t rssi, uint8_t *ephid);

#endif
