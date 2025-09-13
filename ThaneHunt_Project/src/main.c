#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_ble.h"
#include "app_button.h"
#include "app_hid.h"

LOG_MODULE_REGISTER(MAIN);

int main(void)
{
	int err;

	LOG_INF("Starting BLE HIDS keyboard VERSION: [%s]\n\r", CONFIG_PROJECT_VERSION);

	configure_gpio();

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
	err = bt_register_auth_callbacks();
	if (err)
		return 0;
#endif
	hid_init();

	err = enable_bt();
	if (err)
		return 0;

	button_thread_start();
	bool is_led_off = false;

	for (;;)
	{
		if (is_adv)
		{
			is_led_off = false;
			user_led_toggle();
		}
		else
		{
			if (!is_led_off)
			{
				is_led_off = true;
				user_led_turn_off();
			}
		}
		k_msleep(1000);
		/* Battery level simulation */
		bas_notify();
	}
}
