/*
Name : app_sleep

Description :
    Power management helper module for the BLE HID application on Zephyr RTOS.
    Provides an idle timer and work handler to disconnect BLE and transition
    the system into deep sleep (system-off) after inactivity. Also exposes
    utility functions to start and reset the idle timer.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/poweroff.h>

#include "app_ble.h"
#include "app_sleep.h"

#if CONFIG_IMU_LSM6DSO
#include "app_imu.h"
#endif

LOG_MODULE_REGISTER(APP_SLEEP);

static void idle_work_fn(struct k_work *w);
static void idle_timeout(struct k_timer *t);
;

K_WORK_DEFINE(idle_work, idle_work_fn);
K_TIMER_DEFINE(idle_timer, idle_timeout, NULL);

/*
Function : enter_device_sleep

Description :
    Transitions the system to deep sleep (system off). Intended to be called
    after higher-level teardown (e.g., stopping BLE/advertising).

Parameter :
    None

Return :
    void  (does not return)

Example Call :
    enter_device_sleep();
*/
void enter_device_sleep(void)
{
    LOG_INF("Entering deep sleep (system-off)");
    /* Ensure your app stopped BLE/adv, completed DMA, etc. */
    k_sleep(K_MSEC(5000)); /* grace period */
    sys_poweroff(); /* does not return */
}

/*
Function : idle_work_fn

Description :
    Work item that runs on idle timeout: safely disconnects BLE, waits a
    short grace period, then enters system-off.

Parameter :
    w : Pointer to the work item (unused)

Return :
    void

Example Call :
    k_work_submit(&idle_work);
*/
static void idle_work_fn(struct k_work *w)
{
    k_msleep(100);
#if CONFIG_IMU_LSM6DSO
    int err = lsm6dso_accel_gyro_power_down();
    if (err)
    {
        LOG_ERR("Failed to power down LSM6DSO (err: %d)", err);
    }
    LOG_INF("LSM6DSO powered down");
#endif
    LOG_DBG("Idle work: disconnecting BLE and entering deep sleep");
    (void)ble_disconnect_safe(); /* your helper */

    LOG_WRN("No activity -> disconnect + deep sleep");
    enter_device_sleep(); /* calls sys_poweroff() */
}

/*
Function : idle_timeout

Description :
    Timer callback fired when no activity is observed for the configured
    window. Schedules idle_work to disconnect and power down.

Parameter :
    t : Pointer to the Zephyr timer that expired

Return :
    void

Example Call :
    invoked by k_timer after start
*/
static void idle_timeout(struct k_timer *t)
{
    k_work_submit(&idle_work); /* defer to thread context */
}

/*
Function : start_idle_timer

Description :
    Starts the idle timer with a 30-second timeout. When it expires without
    reset, the system will disconnect BLE and enter deep sleep via the
    idle_work_fn.

Parameter :
    None

Return :
    void

Example Call :
    start_idle_timer();
*/
void start_idle_timer(void)
{
    k_timer_start(&idle_timer, K_SECONDS(CONFIG_DEVICE_IDLE_TIMEOUT_SECONDS), K_NO_WAIT);
    LOG_DBG("Idle timer started");
}

/*
Function : stop_idle_timer

Description :
    Stops the idle timer if running. This prevents the system from entering
    deep sleep due to inactivity.

Parameter :
    None

Return :
    void

Example Call :
    stop_idle_timer();
*/
void stop_idle_timer(void)
{
    k_timer_stop(&idle_timer);
    LOG_DBG("Idle timer stopped");
}

/*
Function : reset_idle_timer

Description :
    Stops the current idle timer and restarts it with a fresh 30-second
    timeout. Intended to be called whenever user activity is detected to
    prevent premature sleep.

Parameter :
    None

Return :
    void

Example Call :
    reset_idle_timer();
*/
void reset_idle_timer(void)
{
    stop_idle_timer();
    start_idle_timer();
    LOG_DBG("Idle timer reset");
}