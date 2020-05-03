#include <stdint.h>
#include <string.h>

#include "dp3t.h"
#include "dp3t-config.h"

#define CSTORE_ADDR 0
#define CONTACTS_BASE (CSTORE_ADDR + sizeof(contacts_t))


int cstore_add( uint8_t day, uint8_t epoch, uint8_t rssi, uint8_t *ephid)
{
    contact_t c;
    c.day = day;
    c.epoch = epoch;
    c.rssi = rssi;
    memcpy(c.data, ephid, 16);
    /* TODO: write to store page, update index */
    return 0;
}
