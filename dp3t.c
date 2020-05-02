/* The Fastest Proximity Tracing in the West (FPTW)
 * aka the Secret Pangolin Code
 *
 * Copyright (C) 2020 Dyne.org foundation
 * designed, written and maintained by Daniele Lacamera and Denis Roio
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#ifndef WOLFSSL_USER_SETTINGS
#include <wolfssl/options.h>
#else
#include "user_settings.h"
#endif
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/hmac.h>

#include <dp3t.h>

// zero nonce, one ephid long
const uint8_t zero16[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

char *create_key(WC_RNG* rng) { return NULL; }

// renew the SK in place (reuse input buffer)
void renew_key(sk_t dest, sk_t src) {
	wc_Sha256 sha;
	assert( wc_InitSha256(&sha) == 0);
	wc_Sha256Update(&sha, src, 32);
	wc_Sha256Final(&sha, dest);
	wc_Sha256Free(&sha);
}

// epd = epochs per day = ((24 * 60) / ttl in minutes) +1
int32_t generate_beacons(beacons_t *beacons, uint32_t max_beacons,
                         const sk_t oldest_sk, const uint32_t day, const uint32_t ttl,
                         const char *bk, uint32_t bklen) {
	Aes aes;
	Hmac hmac;
	register uint32_t i;
	sk_t sk, sk_n;
	uint8_t prf[32];

	assert(ttl > 0);
	assert(bk);
	assert(bklen > 0);
	beacons->epochs = (24*60)/ttl+1;
	memcpy(beacons->broadcast, bk, bklen>32?32:bklen);
	beacons->broadcast_len       = bklen;

	memcpy(sk, oldest_sk, 32);
	for (i = 0; i < day; i++) {
		renew_key(sk_n, sk);
		memcpy(sk, sk_n, 32);
	}

	/* PRF */
	wc_HmacInit(&hmac, NULL, INVALID_DEVID);
	wc_HmacSetKey(&hmac, WC_SHA256, sk, 32);
	wc_HmacUpdate(&hmac, (const byte*)beacons->broadcast, beacons->broadcast_len);
	wc_HmacFinal(&hmac, prf);
	wc_HmacFree(&hmac);

	wc_AesInit(&aes, NULL, INVALID_DEVID);
	wc_AesSetKeyDirect(&aes, prf, 32, zero16, AES_ENCRYPTION);
	for(i=0; i<beacons->epochs; i++)
		wc_AesCtrEncrypt(&aes, beacons->ephids[i], zero16, 16);
	wc_AesFree(&aes);
	return(0);
}

int32_t match_positive(matches_t *matches, uint32_t max_matches,
                       const sk_t positive, const contacts_t *contacts) {
	Hmac hmac;
	Aes aes;
	uint8_t prf[32];
	register uint32_t i, ii;
	uint8_t skeph[16];
	register uint32_t ret = 0;

	assert(matches);
	assert(positive);
	assert(contacts);

	// initial buffer allocation: number of ephids / 8
	wc_HmacInit(&hmac, NULL, INVALID_DEVID);
	wc_AesInit(&aes, NULL, INVALID_DEVID);

	wc_HmacSetKey(&hmac, WC_SHA256, positive, 32);
	wc_HmacUpdate(&hmac, (const byte*)contacts->broadcast, contacts->broadcast_len);
	wc_HmacFinal(&hmac, prf);
	wc_AesSetKeyDirect(&aes, prf, 32, zero16, AES_ENCRYPTION);
	// calculate ephids of the current posisk
	for(i=0; i<contacts->epochs; i++) {
		wc_AesCtrEncrypt(&aes, skeph, zero16, 16);
		for(ii=0; ii<contacts->count; ii++) {
			if( memcmp(skeph, contacts->ephids[ii].data, 16) ==0) {
				if(ret<max_matches) {
					matches->ephids[ret] = &contacts->ephids[ii];
					matches->count++;
					ret++;
				} else goto finish;
			}
		}
	}
finish:
	wc_AesFree(&aes);
	wc_HmacFree(&hmac);
	return(ret);
}
