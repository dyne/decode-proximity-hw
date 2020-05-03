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

#include "dp3t-config.h"
#include "dp3t.h"
#include "keystore.h"
#include "ble_scan.h"

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
    { "testvec", "print test vectors", dp3t_shellcmd_testvec },
    { "rekey", "regenerate DP3T secure key", dp3t_shellcmd_rekey },
#ifdef MODULE_WOLFCRYPT_TEST
    { "wolftest", "Perform wolfcrypt porting test", wolftest },
#endif
    { NULL, NULL, NULL }
};

char line_buf[SHELL_DEFAULT_BUFSIZE];

int main(void)
{
    uint8_t *ephid, *sk_t0;

    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    /* Global wolfSSL initialization */
    wolfSSL_Init();
    wolfSSL_Debugging_ON();

    /* dp3t */
    dp3t_start();

    /* Start dp3t gatt advertisements */
    gatt_server();

    /* Start dp3t scan service  */
    dp3t_blescan_init();

    /* start shell */
    printf( "All up, running the shell now\r\n");
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
