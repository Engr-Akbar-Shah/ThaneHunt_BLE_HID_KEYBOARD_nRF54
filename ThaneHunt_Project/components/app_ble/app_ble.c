
#include <string.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/bluetooth/services/bas.h>
#include <bluetooth/services/hids.h>
#include <zephyr/bluetooth/services/dis.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include "app_ble.h"
#include "app_hid.h"

LOG_MODULE_REGISTER(APP_BLE);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

struct k_work pairing_work;

volatile bool is_adv;

K_MSGQ_DEFINE(mitm_queue,
              sizeof(struct pairing_data_mitm),
              CONFIG_BT_HIDS_MAX_CLIENT_COUNT,
              4);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE,
                  (CONFIG_BT_DEVICE_APPEARANCE >> 0) & 0xff,
                  (CONFIG_BT_DEVICE_APPEARANCE >> 8) & 0xff),
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
                  BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

struct conn_mode conn_mode[CONFIG_BT_HIDS_MAX_CLIENT_COUNT];

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

void advertising_start(void)
{
    int err;
    const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
        BT_LE_ADV_OPT_CONN,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);

    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd,
                          ARRAY_SIZE(sd));
    if (err)
    {
        if (err == -EALREADY)
        {
            LOG_INF("Advertising continued\n");
        }
        else
        {
            LOG_INF("Advertising failed to start (err %d)\n", err);
        }

        return;
    }

    is_adv = true;
    LOG_INF("Advertising successfully started\n");
}

void pairing_process()
{
    int err;
    struct pairing_data_mitm pairing_data;

    char addr[BT_ADDR_LE_STR_LEN];

    err = k_msgq_peek(&mitm_queue, &pairing_data);
    if (err)
    {
        return;
    }

    bt_addr_le_to_str(bt_conn_get_dst(pairing_data.conn),
                      addr, sizeof(addr));

    LOG_INF("Passkey for %s: %06u\n", addr, pairing_data.passkey);

    if (IS_ENABLED(CONFIG_SOC_SERIES_NRF54HX) || IS_ENABLED(CONFIG_SOC_SERIES_NRF54LX))
    {
        LOG_INF("Press Button 0 to confirm, Button 1 to reject.\n");
    }
    else
    {
        LOG_INF("Press Button 1 to confirm, Button 2 to reject.\n");
    }
}

void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        LOG_INF("Failed to connect to %s 0x%02x %s\n", addr, err, bt_hci_err_to_str(err));
        return;
    }

    LOG_INF("Connected %s\n", addr);

    err = connect_bt_hid(conn);

    if (err)
    {
        LOG_INF("Failed to notify HID service about connection\n");
        return;
    }

    for (size_t i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++)
    {
        if (!conn_mode[i].conn)
        {
            conn_mode[i].conn = conn;
            conn_mode[i].in_boot_mode = false;
            break;
        }
    }

#if CONFIG_NFC_OOB_PAIRING == 0
    for (size_t i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++)
    {
        if (!conn_mode[i].conn)
        {
            advertising_start();
            return;
        }
    }
#endif
    is_adv = false;
}

void disconnected(struct bt_conn *conn, uint8_t reason)
{
    int err;
    bool is_any_dev_connected = false;
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Disconnected from %s, reason 0x%02x %s\n", addr, reason, bt_hci_err_to_str(reason));

    err = disconnect_bt_hid(conn);

    if (err)
    {
        LOG_INF("Failed to notify HID service about disconnection\n");
    }

    for (size_t i = 0; i < CONFIG_BT_HIDS_MAX_CLIENT_COUNT; i++)
    {
        if (conn_mode[i].conn == conn)
        {
            conn_mode[i].conn = NULL;
        }
        else
        {
            if (conn_mode[i].conn)
            {
                is_any_dev_connected = true;
            }
        }
    }

    advertising_start();
}

void bas_notify(void)
{
    uint8_t battery_level = bt_bas_get_battery_level();

    battery_level--;

    if (!battery_level)
    {
        battery_level = 100U;
    }

    bt_bas_set_battery_level(battery_level);
}

int enable_bt(void)
{
    int err = bt_enable(NULL);
    if (err)
    {
        LOG_INF("Bluetooth init failed (err %d)\n", err);
        return err;
    }
    LOG_INF("Bluetooth initialized\n");

    if (IS_ENABLED(CONFIG_SETTINGS))
    {
        settings_load();
    }
    advertising_start();

    k_work_init(&pairing_work, pairing_process);
    return err;
}

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err)
    {
        LOG_INF("Security changed: %s level %u\n", addr, level);
    }
    else
    {
        LOG_INF("Security failed: %s level %u err %d %s\n", addr, level, err,
               bt_security_err_to_str(err));
    }
}
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
#if (CONFIG_ENABLE_PASS_KEY_AUTH)
    .security_changed = security_changed,
#endif
};

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
    int err;

    struct pairing_data_mitm pairing_data;

    pairing_data.conn = bt_conn_ref(conn);
    pairing_data.passkey = passkey;

    err = k_msgq_put(&mitm_queue, &pairing_data, K_NO_WAIT);
    if (err)
    {
        LOG_INF("Pairing queue is full. Purge previous data.\n");
    }

    /* In the case of multiple pairing requests, trigger
     * pairing confirmation which needed user interaction only
     * once to avoid display information about all devices at
     * the same time. Passkey confirmation for next devices will
     * be proccess from queue after handling the earlier ones.
     */
    if (k_msgq_num_used_get(&mitm_queue) == 1)
    {
        k_work_submit(&pairing_work);
    }
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Pairing cancelled: %s\n", addr);
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Pairing completed: %s, bonded: %d\n", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    struct pairing_data_mitm pairing_data;

    if (k_msgq_peek(&mitm_queue, &pairing_data) != 0)
    {
        return;
    }

    if (pairing_data.conn == conn)
    {
        bt_conn_unref(pairing_data.conn);
        k_msgq_get(&mitm_queue, &pairing_data, K_NO_WAIT);
    }

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Pairing failed conn: %s, reason %d %s\n", addr, reason,
           bt_security_err_to_str(reason));
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .passkey_display = auth_passkey_display,
    .passkey_confirm = auth_passkey_confirm,
    .cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed};

int bt_register_auth_callbacks(void)
{
    int err = bt_conn_auth_cb_register(&conn_auth_callbacks);
    if (err)
    {
        LOG_INF("Failed to register authorization callbacks %d.\n", err);
        return err;
    }
    return err;

    err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
    if (err)
    {
        LOG_INF("Failed to register authorization info callbacks. %d\n", err);
        return err;
    }
    return err;
}

#endif