#include "event/timeout.h"
#include "nimble_scanner.h"
#include "net/bluetil/ad.h"
#include "nimble_scanlist.h"
#include "dp3t-config.h"
#include "dp3t.h"
#include "keystore.h"
#include "contactstore.h"
 

static event_queue_t eq;
static event_t blescan_evt;
static event_timeout_t blescan_timeout_evt;


/*** SCAN ***/
static void dp3t_print_entry(int *idx, nimble_scanlist_entry_t *e)
{
    char name[(BLE_ADV_PDU_LEN + 1)] = { 0 };
    char  peer_ephid[17] = { 0 };
    int res;
    bluetil_ad_t ad = BLUETIL_AD_INIT(e->ad, e->ad_len, e->ad_len);
    res = bluetil_ad_find_str(&ad, BLE_GAP_AD_NAME, name, sizeof(name));
    if (res != BLUETIL_AD_OK) {
        res = bluetil_ad_find_str(&ad, BLE_GAP_AD_NAME_SHORT, name, sizeof(name));
    }
    if (res != BLUETIL_AD_OK) {
        strncpy(name, "undefined", sizeof(name));
    }
    res = bluetil_ad_find_str(&ad, BLE_GAP_AD_UUID128_COMP, peer_ephid, sizeof(peer_ephid));
    if (res == BLUETIL_AD_OK) {
        printf("[rx ephid %02d RSSI %d]: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
                (*idx)++,
                e->last_rssi,
                peer_ephid[0],
                peer_ephid[1],
                peer_ephid[2],
                peer_ephid[3],
                peer_ephid[4],
                peer_ephid[5],
                peer_ephid[6],
                peer_ephid[7],
                peer_ephid[8],
                peer_ephid[9],
                peer_ephid[10],
                peer_ephid[11],
                peer_ephid[12],
                peer_ephid[13],
                peer_ephid[14],
                peer_ephid[15]); 
        cstore_add(0, 0, (uint8_t) ((0 - e->last_rssi) & 0xFF), (uint8_t *)peer_ephid); 
    }
}

void dp3t_scanlist_print(void)
{
    int i = 0;
    nimble_scanlist_entry_t *e = nimble_scanlist_get_next(NULL);
    while (e) {
        dp3t_print_entry(&i, e);
        e = nimble_scanlist_get_next(e);
    }
}

static void blescan(event_t *e)
{
    uint32_t timeout = BLESCAN_DURATION;
    nimble_scanlist_clear();
//    printf("BLE scan\n"); 
    nimble_scanner_start();
    xtimer_usleep(timeout * 1000);
    nimble_scanner_stop();
//    puts("Contacts:");
//    nimble_scanlist_print();
    dp3t_scanlist_print();
    /* schedule next update event */
    event_timeout_set(&blescan_timeout_evt, BLESCAN_INTERVAL * 1000);
}


#define INITIAL_BLESCAN_INTERVAL 2000
static char blescan_thread_stack[THREAD_STACKSIZE_MAIN];

void dp3t_blescan_start(void)
{
    event_timeout_set(&blescan_timeout_evt, INITIAL_BLESCAN_INTERVAL * 1000);
}

static void *blescan_task(void *arg)
{
    (void)arg;
    struct ble_gap_disc_params scan_params = {
        .itvl = BLE_GAP_LIM_DISC_SCAN_INT,
        .window = BLE_GAP_LIM_DISC_SCAN_WINDOW,
        .filter_policy = 0,                         /* don't use */
        .limited = 0,                               /* no limited discovery */
        .passive = 0,                               /* no passive scanning */
        . filter_duplicates = 0,                    /* no duplicate filtering */
    };
    /* initialize the nimble scanner */
    nimble_scanlist_init();
    nimble_scanner_init(&scan_params, nimble_scanlist_update);

    /* initialize the timer to trigger scan events */
    event_queue_init(&eq);
    blescan_evt.handler = blescan;
    event_timeout_init(&blescan_timeout_evt, &eq, &blescan_evt);
    dp3t_blescan_start();
    event_loop(&eq);
    return NULL; /* never reached */
}


void dp3t_blescan_init(void)
{
    thread_create(blescan_thread_stack, THREAD_STACKSIZE_MAIN,
            THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
            blescan_task, NULL, "blescan");
}

void dp3t_blescan_stop(void)
{
    event_timeout_clear(&blescan_timeout_evt);
}
