#ifndef APP_HID_H
#define APP_HID_H

struct keyboard_state;

void hid_init(void);
int connect_bt_hid(struct bt_conn *conn);
int disconnect_bt_hid(struct bt_conn *conn);
int key_report_con_send(const struct keyboard_state *state,
						bool boot_mode,
						struct bt_conn *conn);
int hid_buttons_release(const uint8_t *keys, size_t cnt);
int hid_buttons_press(const uint8_t *keys, size_t cnt);

#endif // APP_HID_H
