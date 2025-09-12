#include "app_imu.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(APP_IMU, LOG_LEVEL_INF);

// --- LSM6DSO I2C address and register definitions ---
#define LSM6DSO_I2C_ADDR 0x6A // LSM6DSO I2C device address

#define LSM6DSO_REG_WHO_AM_I 0x0F // Identification register
#define LSM6DSO_WHO_AM_I_VAL 0x6A // Expected WHO_AM_I value

#define LSM6DSO_REG_CTRL1_XL 0x10 // Accelerometer control register
#define LSM6DSO_REG_CTRL2_G 0x11  // Gyroscope control register
// Accelerometer/gyroscope data output registers (low byte first)
#define LSM6DSO_REG_OUTX_L_XL 0x28 // Accelerometer X axis low byte
#define LSM6DSO_REG_OUTX_L_G 0x22  // Gyroscope X axis low byte

#define IMU_THREAD_STACK_SIZE 2048

static struct k_thread imu_thread_data;
static k_tid_t imu_thread_tid;
K_THREAD_STACK_DEFINE(imu_thread_stack, IMU_THREAD_STACK_SIZE);

struct lsm6dso_raw_data
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

static int lsm6dso_i2c_reg_write_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t value)
{
    uint8_t tx_buf[2] = {reg_addr, value};
    return i2c_write(i2c_dev, tx_buf, sizeof(tx_buf), LSM6DSO_I2C_ADDR);
}

static int lsm6dso_i2c_reg_read_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *value)
{
    return i2c_reg_read_byte(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, value);
}

static int lsm6dso_i2c_reg_read_bytes(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    return i2c_burst_read(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, data, len);
}

static int lsm6dso_fetch_raw_data(const struct device *i2c_dev, struct lsm6dso_raw_data *raw_data_out)
{
    uint8_t accel_data[6];
    uint8_t gyro_data[6];
    int ret;

    // Read accelerometer data (6 bytes)
    ret = lsm6dso_i2c_reg_read_bytes(i2c_dev, LSM6DSO_REG_OUTX_L_XL, accel_data, 6);
    if (ret != 0)
    {
        LOG_ERR("Failed to read accelerometer data (err: %d).", ret);
        return ret;
    }
    // Raw data is 16-bit signed integer, low byte first
    raw_data_out->accel_x = (int16_t)(accel_data[0] | (accel_data[1] << 8));
    raw_data_out->accel_y = (int16_t)(accel_data[2] | (accel_data[3] << 8));
    raw_data_out->accel_z = (int16_t)(accel_data[4] | (accel_data[5] << 8));

    // Read gyroscope data (6 bytes)
    ret = lsm6dso_i2c_reg_read_bytes(i2c_dev, LSM6DSO_REG_OUTX_L_G, gyro_data, 6);
    if (ret != 0)
    {
        LOG_ERR("Failed to read gyroscope data (err: %d).", ret);
        return ret;
    }
    // Raw data is 16-bit signed integer, low byte first
    raw_data_out->gyro_x = (int16_t)(gyro_data[0] | (gyro_data[1] << 8));
    raw_data_out->gyro_y = (int16_t)(gyro_data[2] | (gyro_data[3] << 8));
    raw_data_out->gyro_z = (int16_t)(gyro_data[4] | (gyro_data[5] << 8));

    return 0;
}

static void lsm6dso_display_raw_data(const struct lsm6dso_raw_data *raw_data, int count)
{
    LOG_INF("accel raw: X:%d Y:%d Z:%d (LSB)\n",
           raw_data->accel_x, raw_data->accel_y, raw_data->accel_z);
    LOG_INF("gyro raw: X:%d Y:%d Z:%d (LSB)\n",
           raw_data->gyro_x, raw_data->gyro_y, raw_data->gyro_z);
    LOG_INF("trig_cnt:%d\n\n", count);
}

static int lsm6dso_init(const struct device *i2c_dev)
{
    uint8_t who_am_i = 0;
    int ret;

    // Verify device ID
    ret = lsm6dso_i2c_reg_read_byte(i2c_dev, LSM6DSO_REG_WHO_AM_I, &who_am_i);
    if (ret != 0)
    {
        LOG_ERR("Failed to read WHO_AM_I register (err: %d)", ret);
        return ret;
    }
    if (who_am_i != LSM6DSO_WHO_AM_I_VAL)
    {
        LOG_ERR("Invalid WHO_AM_I: 0x%02x, expected 0x%02x", who_am_i, LSM6DSO_WHO_AM_I_VAL);
        return -ENODEV;
    }
    LOG_INF("LSM6DSO WHO_AM_I check passed. ID: 0x%02x", who_am_i);

    // Set accelerometer ODR (12.5 Hz) and 2g range (0x20)
    ret = lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL1_XL, 0x20);
    if (ret != 0)
    {
        LOG_ERR("Failed to set CTRL1_XL register (err: %d)", ret);
        return ret;
    }

    // Set gyroscope ODR (12.5 Hz) and 250dps range (0x20)
    ret = lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL2_G, 0x20);
    if (ret != 0)
    {
        LOG_ERR("Failed to set CTRL2_G register (err: %d)", ret);
        return ret;
    }

    LOG_INF("LSM6DSO initialized successfully.");
    return 0;
}

void imu_thread_fn(void *p1, void *p2, void *p3)
{
    const struct device *i2c_dev = DEVICE_DT_GET(DT_ALIAS(lsm6ds0i2c));
    struct lsm6dso_raw_data sensor_data;
    static int trig_cnt = 0; // Ensure only initialized once in main scope

    if (!device_is_ready(i2c_dev))
    {
        LOG_ERR("I2C device %s is not ready!", i2c_dev->name);
        return;
    }
    LOG_INF("I2C device %s is ready.", i2c_dev->name);

    if (lsm6dso_init(i2c_dev) != 0)
    {
        LOG_ERR("Failed to initialize LSM6DSO sensor.");
        return;
    }

    LOG_INF("Testing LSM6DSO sensor in polling mode (custom I2C driver) - Raw Data Output.\n\n");

    while (1)
    {
        trig_cnt++; // Increment counter at the start of each loop

        // Fetch raw data
        if (lsm6dso_fetch_raw_data(i2c_dev, &sensor_data) == 0)
        {
            // Display raw data
            lsm6dso_display_raw_data(&sensor_data, trig_cnt);
        }
        else
        {
            LOG_ERR("Failed to fetch data.");
        }

        k_sleep(K_MSEC(1000)); // Read once every second
    }

    return;
}

void imu_thread_start(void)
{
    /* Don’t start twice */
    if (imu_thread_tid)
    {
        return;
    }

    imu_thread_tid = k_thread_create(
        &imu_thread_data,
        imu_thread_stack,
        K_THREAD_STACK_SIZEOF(imu_thread_stack),
        imu_thread_fn,
        NULL, NULL, NULL,
        0,        /* priority: same as your K_THREAD_DEFINE (0) */
        0,        /* options */
        K_NO_WAIT /* start immediately */
    );

    /* Optional: give it a name for debugging */
    k_thread_name_set(imu_thread_tid, "IMU_LSM60_THREAD");
}