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

#include "shell.h"
#include "msg.h"
#include "nimble_scanner.h"
#include "net/bluetil/ad.h"
#include "nimble_scanlist.h"
#include "dp3t.h"


/*** SCAN ***/

/* default scan duration (1s) */
#define DEFAULT_DURATION        (1000000U)

void dp3t_print_entry(int *idx, nimble_scanlist_entry_t *e)
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
        printf("[%02d] DP-3T: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                (*idx)++,
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
        /* TODO: receive ephid */
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

int cmd_scan(int argc, char **argv)
{
    uint32_t timeout = DEFAULT_DURATION;

    if ((argc == 2) && (memcmp(argv[1], "help", 4) == 0)) {
        printf("usage: %s [timeout in ms]\n", argv[0]);
        return 0;
    }
    if (argc >= 2) {
        timeout = (uint32_t)(atoi(argv[1]) * 1000);
    }

    nimble_scanlist_clear();
    printf("Scanning for %ums now ...", (unsigned)(timeout / 1000));
    nimble_scanner_start();
    xtimer_usleep(timeout);
    nimble_scanner_stop();
    puts(" done\n\nResults:");
    nimble_scanlist_print();
    dp3t_scanlist_print();
    puts("");
    return 0;
}

int cmd_testvec(int argc, char **argv)
{
    uint8_t testkey[32] = { }; /* zeros */
    dp3t_create_ephids(testkey);
    return 0;
}

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int dtls_client(int argc, char **argv);
extern int dtls_server(int argc, char **argv);

#ifdef MODULE_WOLFCRYPT_TEST
extern int wolfcrypt_test(void* args);
static int wolftest(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    wolfcrypt_test(NULL);
    return 0;
}
#endif
extern int gatt_server(void);

static const shell_command_t shell_commands[] = {
    { "dtlsc", "Start a DTLS client", dtls_client },
    { "dtlss", "Start and stop a DTLS server", dtls_server },
    { "scan", "trigger a BLE scan", cmd_scan },
    { "testvec", "print test vectors", cmd_testvec },
#ifdef MODULE_WOLFCRYPT_TEST
    { "wolftest", "Perform wolfcrypt porting test", wolftest },
#endif
    { NULL, NULL, NULL }
};


int main(void)
{
    uint8_t *ephid, *sk_t0;
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
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    wolfSSL_Init();
    wolfSSL_Debugging_ON();

    /* dp3t */
    sk_t0 = dp3t_get_skt_0();
    dp3t_create_ephids(sk_t0);

    /* Start Bluetooth service by default */
    gatt_server();

    /* start shell */
    printf( "All up, running the shell now\r\n");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    /* should be never reached */
    return 0;
}
