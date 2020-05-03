#ifndef DP3T_CONFIG_H
#define DP3T_CONFIG_H
#include <stdint.h>

/* SK / EPHID / SHA base parameters
 */
#define SK_LEN 32
#define SHA256_LEN 32
#define EPHID_LEN 16


/* Use-case DP3-T parameters */
#define TTL (180) // In minutes
#define EPOCHS  (((24 * 60) / TTL) + 1)
#define RETENTION_PERIOD (14) // In days
#define BROADCAST_KEY "Broadcast key"
#define BROADCAST_KEY_LEN 13UL

/* Bluetooth scan/adv timing (in milliseconds) */
#define BLESCAN_INTERVAL 58000
#define BLESCAN_DURATION 2000

#endif
