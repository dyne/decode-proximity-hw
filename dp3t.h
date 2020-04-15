/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License (GPL) version 2. See the file LICENSE
 * for more details.
 *
 * @}
 */
#ifndef DP3T_H_INCLUDED
#define DP3T_H_INCLUDED

#define SK_LEN 32
#define SHA256_LEN 32
#define EPHID_LEN 16

uint8_t *dp3t_get_skt_0(void);
void    dp3t_get_skt_1(const uint8_t *skt_0, uint8_t *skt_1);
void    dp3t_create_ephids(void);
uint8_t *dp3t_get_ephid(int epoch);

#endif
