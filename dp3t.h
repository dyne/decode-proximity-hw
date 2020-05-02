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

#ifndef __DP3T_H__
#define __DP3T_H__

#include <inttypes.h>

typedef uint8_t beacon_t[16];
typedef uint8_t sk_t[32];

// simple offset structure of num elements sized EPHID_LEN bytes
typedef struct {
	uint32_t epochs;           //< data length (capacity) provided by caller
	char     broadcast[32];   //< broadcast key
	uint32_t broadcast_len;  //< broadcast key length
	beacon_t ephids[0];    //< data offset provided by caller
} beacons_t;

typedef struct __attribute__((packed)) {
	uint8_t day;
	uint8_t epoch;
	uint8_t rssi;
	uint8_t reserved;
	uint8_t data[16];
} contact_t;

typedef struct {
	uint32_t count;             //< number of ephids stored
	contact_t *ephids;         //< array of ephids
	uint32_t epochs;          //< how many epochs in a day
	char     broadcast[32];  //< broadcast key
	uint32_t broadcast_len; //< broadcast key length
} contacts_t; // always const

// struct filled with match_positive results, it does not use more
// memory but returns pointers to contact_t data passed in from
// contacts_t
typedef struct {
	uint32_t count;           //< number of ephids stored
	contact_t *ephids[0];     //< array of pointers to contacts found
} matches_t;

void renew_key(sk_t dest, sk_t src);

int32_t generate_beacons(beacons_t *beacons, uint32_t max_beacons,
                         const sk_t oldest_sk, const uint32_t day, const uint32_t ttl,
                         const char *bk, uint32_t bklen);

int32_t match_positive(matches_t *matches, uint32_t max_matches,
                       const sk_t positive, const contacts_t *contacts);


#endif
