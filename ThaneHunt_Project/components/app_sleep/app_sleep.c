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
#include <zephyr/drivers/hwinfo.h>

#include "app_ble.h"
#include "app_sleep.h"

#if CONFIG_IMU_LSM6DSO
#include "app_imu.h"
#endif

LOG_MODULE_REGISTER(APP_SLEEP);

static void idle_work_fn(struct k_work *w);
static void idle_timeout(struct k_timer *t);

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
    LOG_DBG("Entering deep sleep (system-off)");
    /* Ensure your app stopped BLE/adv, completed DMA, etc. */
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
#if CONFIG_IMU_LSM6DSO
    lsm6dso_accel_gyro_power_down();
#endif
    LOG_DBG("Idle work: disconnecting BLE and entering deep sleep");
    (void)ble_disconnect_safe(); /* your helper */
    k_sleep(K_MSEC(3000));       /* grace period */
    enter_device_sleep();        /* calls sys_poweroff() */
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
    LOG_WRN("No activity -> disconnect + deep sleep");
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

void PRINT_RESET_CAUSE()
{
    uint32_t reset_cause = 0;
    if (0 == hwinfo_get_reset_cause(&reset_cause))
    {
        char reset_cause_as_string[16][25] = {
            "RESET BY PIN            ",
            "RESET BY SOFTWARE       ",
            "RESET BY BROWNOUT       ",
            "RESET BY POR            ",
            "RESET BY WATCHDOG       ",
            "RESET BY DEBUG          ",
            "RESET BY SECURITY       ",
            "RESET BY LOW_POWER_WAKE ",
            "RESET BY CPU_LOCKUP     ",
            "RESET BY PARITY         ",
            "RESET BY PLL            ",
            "RESET BY CLOCK          ",
            "RESET BY HARDWARE       ",
            "RESET BY USER           ",
            "RESET BY TEMPERATURE    ",
            "RESET CAUSE UNKNOWN     "};

        LOG_WRN("Reset cause: %d", reset_cause);
        if (0 == reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[15]);
        }
        else if (RESET_PIN & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[0]);
        }
        else if (RESET_SOFTWARE & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[1]);
        }
        else if (RESET_SOFTWARE & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[1]);
        }
        else if (RESET_BROWNOUT & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[2]);
        }
        else if (RESET_POR & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[3]);
        }
        else if (RESET_WATCHDOG & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[4]);
        }
        else if (RESET_DEBUG & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[5]);
        }
        else if (RESET_SECURITY & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[6]);
        }
        else if (RESET_LOW_POWER_WAKE & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[7]);
        }
        else if (RESET_CPU_LOCKUP & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[8]);
        }
        else if (RESET_PARITY & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[9]);
        }
        else if (RESET_PLL & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[10]);
        }
        else if (RESET_CLOCK & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[11]);
        }
        else if (RESET_HARDWARE & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[12]);
        }
        else if (RESET_USER & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[13]);
        }
        else if (RESET_TEMPERATURE & reset_cause)
        {
            LOG_WRN("%s", reset_cause_as_string[14]);
        }
    }
}