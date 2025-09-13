
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <bluetooth/services/hids.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/poweroff.h>

#include "app_ble.h"
#include "app_button.h"
#include "app_hid.h"
#include "app_keycodes.h"

LOG_MODULE_REGISTER(APP_BUTTON, CONFIG_BUTTON_LOG_LEVEL);

#define USER_LED_NODE DT_NODELABEL(led_0)
#define USER_BUTTON_NODE DT_NODELABEL(button_0)
#define USER_BUTTON1_NODE DT_NODELABEL(button_1)

#define USER_BUTTON_PIN DT_GPIO_PIN(USER_BUTTON_NODE, gpios)
#define USER_BUTTON1_PIN DT_GPIO_PIN(USER_BUTTON1_NODE, gpios)

#define KEY_TEXT_MASK BIT(USER_BUTTON_PIN)
#define KEY_SHIFT_MASK BIT(USER_BUTTON1_PIN)

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
#define KEY_PAIRING_ACCEPT BIT(USER_BUTTON_PIN)
#define KEY_PAIRING_REJECT BIT(USER_BUTTON1_PIN)
#endif

#define BUTTON_THREAD_STACK_SIZE 2048
#define BUTTON_THREAD_PRIO 0

#define BUTTON_DEBOUNCE_MS 10

static const struct gpio_dt_spec user_led = GPIO_DT_SPEC_GET(USER_LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(USER_BUTTON_NODE, gpios);
static struct gpio_callback button_cb;

static struct k_thread button_thread_data;
static k_tid_t button_thread_tid;
K_THREAD_STACK_DEFINE(button_thread_stack, BUTTON_THREAD_STACK_SIZE);

K_MSGQ_DEFINE(Button_queue, sizeof(bool), 16, 4);

static void button_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(button_work, button_work_handler);

static volatile uint32_t BUTTON_PIN;

static void idle_timeout(struct k_timer *t);
K_TIMER_DEFINE(idle_timer, idle_timeout, NULL);

void enter_system_off(void)
{
	/* Ensure your app stopped BLE/adv, completed DMA, etc. */
	sys_poweroff(); /* does not return */
}

static void idle_work_fn(struct k_work *w);
K_WORK_DEFINE(idle_work, idle_work_fn);

static void idle_work_fn(struct k_work *w)
{
	(void)ble_disconnect_safe(); /* your helper */
	k_sleep(K_MSEC(3000));		 /* grace period */
	enter_system_off();			 /* calls sys_poweroff() */
}

static void idle_timeout(struct k_timer *t)
{
	LOG_INF("No activity -> disconnect + deep sleep");
	k_work_submit(&idle_work); /* defer to thread context */
}

void num_comp_reply(bool accept)
{
	struct pairing_data_mitm pairing_data;
	struct bt_conn *conn;

	if (k_msgq_get(&mitm_queue, &pairing_data, K_NO_WAIT) != 0)
	{
		return;
	}

	conn = pairing_data.conn;
#if (CONFIG_ENABLE_PASS_KEY_AUTH)
	if (accept)
	{
		bt_conn_auth_passkey_confirm(conn);
		LOG_INF("Numeric Match, conn %p\n", conn);
	}
	else
	{
		bt_conn_auth_cancel(conn);
		LOG_ERR("Numeric Reject, conn %p\n", conn);
	}
#endif
	bt_conn_unref(pairing_data.conn);

	if (k_msgq_num_used_get(&mitm_queue))
	{
		k_work_submit(&pairing_work);
	}
}

static void button_text_changed(bool down, uint8_t *chr)
{
	if (down)
	{
		(void)hid_buttons_press(chr, 1);
	}
	else
	{
		(void)hid_buttons_release(chr, 1);
	}
}

static void button_shift_changed(bool down)
{
	static const uint8_t shift_key = HID_KEY_LSHIFT;

	if (down)
	{
		(void)hid_buttons_press(&shift_key, 1);
	}
	else
	{
		(void)hid_buttons_release(&shift_key, 1);
	}
}

static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	BUTTON_PIN = pins;
	k_work_reschedule(&button_work, K_MSEC(BUTTON_DEBOUNCE_MS));
}

static void button_work_handler(struct k_work *work)
{
	bool gp_state = gpio_pin_get_dt(&button);
	(void)k_msgq_put(&Button_queue, &gp_state, K_NO_WAIT);
}

int init_user_led(void)
{
	int err;
	err = device_is_ready(user_led.port);
	if (!err)
	{
		LOG_ERR("LED device not ready\n");
		return err;
	}

	err = gpio_pin_configure_dt(&user_led, GPIO_OUTPUT);
	if (err)
	{
		LOG_ERR("Error %d: failed to configure LED pin\n", err);
		return err;
	}
	return err;
}

void user_led_turn_on(void)
{
	if (!gpio_pin_set_dt(&user_led, 1))
	{
		LOG_INF("User LED Turn ON\n\r");
	}
}

void user_led_turn_off(void)
{
	if (!gpio_pin_set_dt(&user_led, 0))
	{
		LOG_INF("User LED Turn OFF\n\r");
	}
}

void user_led_toggle(void)
{
	gpio_pin_toggle_dt(&user_led);
}

void configure_gpio(void)
{
	int ret;

	if (!device_is_ready(button.port))
	{
		LOG_ERR("Error: button device not ready\n");
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret)
	{
		LOG_ERR("Error %d: failed to configure button pin\n", ret);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret)
	{
		LOG_ERR("Error %d: failed to configure interrupt\n", ret);
		return;
	}

	gpio_init_callback(&button_cb, button_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb);

	/* LEDs init (unchanged) */
	int err = init_user_led();
	if (err)
	{
		LOG_ERR("Cannot init LEDs (err: %d)\n", err);
	}

	/* Start the consumer thread */
	button_thread_start();

	LOG_INF("Button configured: port=%s pin=%u active_%s, debounce=%dms\n",
			button.port->name, button.pin,
			(button.dt_flags & GPIO_ACTIVE_LOW) ? "low" : "high",
			BUTTON_DEBOUNCE_MS);
}

static void button_thread_fn(void *p1, void *p2, void *p3)
{
	bool ev;

	k_timer_start(&idle_timer, K_SECONDS(30), K_NO_WAIT);

	for (;;)
	{
		k_msgq_get(&Button_queue, &ev, K_FOREVER);

		/* Any activity -> restart idle timer */
		k_timer_stop(&idle_timer);
		k_timer_start(&idle_timer, K_SECONDS(30), K_NO_WAIT);

		if (BUTTON_PIN & KEY_TEXT_MASK)
		{
			static uint8_t key = HID_KEY_H;
			button_text_changed(ev, &key);
		}
		else if (BUTTON_PIN & KEY_SHIFT_MASK)
		{
			button_shift_changed(ev);
		}
		k_msleep(1000);
	}
}

void button_thread_start(void)
{
	if (button_thread_tid)
	{
		return; /* already started */
	}

	button_thread_tid = k_thread_create(
		&button_thread_data,
		button_thread_stack,
		K_THREAD_STACK_SIZEOF(button_thread_stack),
		button_thread_fn,
		NULL, NULL, NULL,
		BUTTON_THREAD_PRIO, 0, K_NO_WAIT);

	k_thread_name_set(button_thread_tid, "btn_thread");
}

int read_latch_register(void)
{
	uint32_t lat = NRF_P1->LATCH; /* snapshot */
	if (lat)
	{
		LOG_INF("LATCH P1: 0x%08x", lat);
		if (lat & BIT(USER_BUTTON_PIN))
		{
			LOG_INF("Button (P1.%u) was the wakeup source", USER_BUTTON_PIN);
		}
		/* Clear only the bits that were set */
		NRF_P0->LATCH = lat; /* write-1-to-clear */
	}
	else
	{
		LOG_INF("No GPIO on P1 caused the wakeup");
	}

	/* If your board also uses P1, read/clear NRF_P1->LATCH similarly. */
	return 0;
}

SYS_INIT(read_latch_register, PRE_KERNEL_1, 0);