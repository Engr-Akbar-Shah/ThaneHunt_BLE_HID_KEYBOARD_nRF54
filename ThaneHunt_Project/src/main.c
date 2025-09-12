#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include "app_ble.h"
#include "app_button.h"
#include "app_hid.h"

LOG_MODULE_REGISTER(MAIN);

#define MODIFIER_KEY_POS 0
#define SHIFT_KEY_CODE 0x02

#define ADV_LED_BLINK_INTERVAL 1000

#define ADV_STATUS_LED DK_LED1

/* HIDs queue elements. */
#define HIDS_QUEUE_SIZE 10

/* ********************* */
/* Buttons configuration */

/* Note: The configuration below is the same as BOOT mode configuration
 * This simplifies the code as the BOOT mode is the same as REPORT mode.
 * Changing this configuration would require separate implementation of
 * BOOT mode report generation.
 */

#define KEY_CODE_MIN 0	 /* Normal key codes */
#define KEY_CODE_MAX 101 /* Normal key codes */


#define KEY_CTRL_CODE_MIN 224 /* Control key codes - required 8 of them */
#define KEY_CTRL_CODE_MAX 231 /* Control key codes - required 8 of them */

/* Current report map construction requires exactly 8 buttons */
BUILD_ASSERT((KEY_CTRL_CODE_MAX - KEY_CTRL_CODE_MIN) + 1 == 8);

int main(void)
{
	int err;
	int blink_status = 0;

	LOG_INF("Starting BLE HIDS keyboard VERSION: [%s]", CONFIG_PROJECT_VERSION);

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

	for (;;)
	{
		if (is_adv)
		{
			dk_set_led(ADV_STATUS_LED, (++blink_status) % 2);
		}
		else
		{
			dk_set_led_off(ADV_STATUS_LED);
		}
		k_sleep(K_MSEC(ADV_LED_BLINK_INTERVAL));
		/* Battery level simulation */
		bas_notify();
	}
}
