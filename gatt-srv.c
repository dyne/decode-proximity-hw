/*
 * Copyright (C) 2020 Dyne.org foundation
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License (GPL) version 2. See the file LICENSE
 * for more details.
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>

#include "assert.h"
#include "event/timeout.h"
#include "nimble_riot.h"
#include "net/bluetil/ad.h"

#include "host/ble_hs.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "dp3t.h"

#define HRS_FLAGS_DEFAULT       (0x01)      /* 16-bit BPM value */
#define SENSOR_LOCATION         (0x02)      /* wrist sensor */
#define UPDATE_INTERVAL         (250U * US_PER_MS)

static const char *device_name = "DP3T";
static const char *_manufacturer_name = "Dyne.org";
static const char *_model_number = "1";

static event_queue_t _eq;
static event_t _update_evt;

static uint16_t _conn_handle;
static uint16_t _hrs_val_handle;


static int _devinfo_handler(uint16_t conn_handle, uint16_t attr_handle,
                            struct ble_gatt_access_ctxt *ctxt, void *arg);

static void start_advertising(void);

/* GATT service definitions */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /* Device Information Service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_GATT_SVC_DEVINFO),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_MANUFACTURER_NAME),
            .access_cb = _devinfo_handler,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_MODEL_NUMBER_STR),
            .access_cb = _devinfo_handler,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_SERIAL_NUMBER_STR),
            .access_cb = _devinfo_handler,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_FW_REV_STR),
            .access_cb = _devinfo_handler,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_HW_REV_STR),
            .access_cb = _devinfo_handler,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* no more characteristics in this service */
        }, }
    },
    {
        0, /* no more services */
    },
};

static int _devinfo_handler(uint16_t conn_handle, uint16_t attr_handle,
                            struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)arg;
    const char *str;

    switch (ble_uuid_u16(ctxt->chr->uuid)) {
        case BLE_GATT_CHAR_MANUFACTURER_NAME:
            puts("[READ] device information service: manufacturer name value");
            str = _manufacturer_name;
            break;
        case BLE_GATT_CHAR_MODEL_NUMBER_STR:
            puts("[READ] device information service: model number value");
            str = _model_number;
            break;
        default:
            return BLE_ATT_ERR_UNLIKELY;
    }

    int res = os_mbuf_append(ctxt->om, str, strlen(str));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    /* Used only if we want to export other GATT services */
    return 0;
}

static void start_advertising(void)
{
    struct ble_gap_adv_params advp;
    int res;

    memset(&advp, 0, sizeof advp);
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;
    advp.itvl_min  = BLE_GAP_ADV_FAST_INTERVAL1_MIN;
    advp.itvl_max  = BLE_GAP_ADV_FAST_INTERVAL1_MAX;
    res = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                            &advp, gap_event_cb, NULL);
    assert(res == 0);
    (void)res;
}

int gatt_server(void)
{
    int res = 0;
    uint8_t buf[31];
//    uint16_t hdr_uuid = BLE_GAP_AD_UUID128_INCOMP;
    uint16_t hdr_uuid = 0xFF;

    puts("BLE DP3T service started");
    /* verify and add our custom services */
    res = ble_gatts_count_cfg(gatt_svr_svcs);
    assert(res == 0);
    res = ble_gatts_add_svcs(gatt_svr_svcs);
    assert(res == 0);

    /* set the device name */
    ble_svc_gap_device_name_set(device_name);
    /* reload the GATT server to link our added services */
    ble_gatts_start();

    /* configure and set the advertising data */
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, device_name);
    bluetil_ad_add(&ad, hdr_uuid, dp3t_get_ephid(0), EPHID_LEN);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    /* start to advertise this node */
    start_advertising();

    /* run an event loop for handling the heart rate update events */
    //event_loop(&_eq);

    return 0;
}
