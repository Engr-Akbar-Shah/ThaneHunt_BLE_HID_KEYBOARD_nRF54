/*
Name : app_imu

Description :
    LSM6DSO IMU interface for Zephyr RTOS over I2C. This module initializes
    the sensor, periodically reads raw accelerometer/gyroscope samples in a
    dedicated thread, and logs the raw LSB values for debugging/bring-up.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

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

#define LSM6DSO_ODR_MASK 0xF0 /* ODR bits are [7:4]; 0000 = power-down */

const struct device *i2c_dev = DEVICE_DT_GET(DT_ALIAS(lsm6ds0i2c));

bool imu_power_down = false;

struct lsm6dso_raw_data
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

/*
Function : lsm6dso_i2c_reg_write_byte

Description :
    Writes a single byte to a given LSM6DSO register over I2C.

Parameter :
    i2c_dev  : Pointer to the I2C device instance
    reg_addr : Target register address
    value    : Byte value to write

Return :
    int : 0 on success, negative errno on failure

Example Call :
    int ret = lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL1_XL, 0x20);
*/
static int lsm6dso_i2c_reg_write_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t value)
{
    uint8_t tx_buf[2] = {reg_addr, value};
    return i2c_write(i2c_dev, tx_buf, sizeof(tx_buf), LSM6DSO_I2C_ADDR);
}

/*
Function : lsm6dso_i2c_reg_read_byte

Description :
    Reads a single byte from a given LSM6DSO register over I2C.

Parameter :
    i2c_dev  : Pointer to the I2C device instance
    reg_addr : Register address to read
    value    : Output pointer to store the read byte

Return :
    int : 0 on success, negative errno on failure

Example Call :
    uint8_t who;
    int ret = lsm6dso_i2c_reg_read_byte(i2c_dev, LSM6DSO_REG_WHO_AM_I, &who);
*/
static int lsm6dso_i2c_reg_read_byte(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *value)
{
    return i2c_reg_read_byte(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, value);
}

/*
Function : lsm6dso_i2c_reg_read_bytes

Description :
    Reads multiple consecutive bytes starting at the given LSM6DSO register.

Parameter :
    i2c_dev  : Pointer to the I2C device instance
    reg_addr : First register address to read from
    data     : Buffer to store read bytes
    len      : Number of bytes to read

Return :
    int : 0 on success, negative errno on failure

Example Call :
    uint8_t buf[6];
    int ret = lsm6dso_i2c_reg_read_bytes(i2c_dev, LSM6DSO_REG_OUTX_L_XL, buf, 6);
*/
static int lsm6dso_i2c_reg_read_bytes(const struct device *i2c_dev, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    return i2c_burst_read(i2c_dev, LSM6DSO_I2C_ADDR, reg_addr, data, len);
}

/*
Function : lsm6dso_fetch_raw_data

Description :
    Fetches raw 16-bit accelerometer and gyroscope samples (XYZ each) from
    the LSM6DSO and packs them into the provided struct. Data is read in LSB
    (little-endian) order.

Parameter :
    i2c_dev      : Pointer to the I2C device instance
    raw_data_out : Output pointer to lsm6dso_raw_data to fill with samples

Return :
    int : 0 on success, negative errno on failure

Example Call :
    struct lsm6dso_raw_data s;
    int ret = lsm6dso_fetch_raw_data(i2c_dev, &s);
*/
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

/*
Function : lsm6dso_display_raw_data

Description :
    Logs the raw accelerometer and gyroscope samples to the Zephyr logger,
    along with a running trigger counter.

Parameter :
    raw_data : Pointer to filled lsm6dso_raw_data with latest samples
    count    : Sequence counter to print with the sample

Return :
    void

Example Call :
    lsm6dso_display_raw_data(&sensor_data, trig_cnt);
*/
static void lsm6dso_display_raw_data(const struct lsm6dso_raw_data *raw_data)
{
    LOG_INF("LSM6DSO ACCEL + GYRP: [AX:%d AY:%d AZ:%d] [GX:%d GY:%d GZ:%d]",
            raw_data->accel_x, raw_data->accel_y, raw_data->accel_z, raw_data->gyro_x, raw_data->gyro_y, raw_data->gyro_z);
            k_msleep(500); // Slow down logging for readability
}

/*
Function : lsm6dso_accel_power_down

Description :
    Powers down the accelerometer by clearing ODR_XL[3:0] (bits 7:4) in CTRL1_XL,
    preserving other configuration bits.

Parameter :
    i2c_dev : Pointer to the I2C device instance

Return :
    int : 0 on success, negative errno on failure

Example Call :
    lsm6dso_accel_power_down(i2c_dev);
*/
static int lsm6dso_accel_power_down(const struct device *i2c_dev)
{
    int ret;
    uint8_t ctrl1;

    ret = lsm6dso_i2c_reg_read_byte(i2c_dev, LSM6DSO_REG_CTRL1_XL, &ctrl1);
    if (ret)
    {
        return ret;
    }
    ctrl1 &= ~LSM6DSO_ODR_MASK; /* ODR_XL = 0000 -> power-down */
    return lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL1_XL, ctrl1);
}

/*
Function : lsm6dso_gyro_power_down

Description :
    Powers down the gyroscope by clearing ODR_G[3:0] (bits 7:4) in CTRL2_G,
    preserving other configuration bits.

Parameter :
    i2c_dev : Pointer to the I2C device instance

Return :
    int : 0 on success, negative errno on failure

Example Call :
    lsm6dso_gyro_power_down(i2c_dev);
*/
static int lsm6dso_gyro_power_down(const struct device *i2c_dev)
{
    int ret;
    uint8_t ctrl2;

    ret = lsm6dso_i2c_reg_read_byte(i2c_dev, LSM6DSO_REG_CTRL2_G, &ctrl2);
    if (ret)
    {
        return ret;
    }
    ctrl2 &= ~LSM6DSO_ODR_MASK; /* ODR_G = 0000 -> power-down */
    return lsm6dso_i2c_reg_write_byte(i2c_dev, LSM6DSO_REG_CTRL2_G, ctrl2);
}

/*
Function : lsm6dso_accel_gyro_power_down

Description :
    Convenience helper to power down both accelerometer and gyroscope by
    setting their ODR fields to 0 (power-down state).

Parameter :
    i2c_dev : Pointer to the I2C device instance

Return :
    int : 0 on success, negative errno on first failure

Example Call :
    lsm6dso_accel_gyro_power_down(i2c_dev);
*/
int lsm6dso_accel_gyro_power_down(void)
{
    int ret;
    imu_power_down = true;

    ret = lsm6dso_accel_power_down(i2c_dev);
    if (ret)
        return ret;

    ret = lsm6dso_gyro_power_down(i2c_dev);
    if (ret)
        return ret;

    return 0;
}

/*
Function : imu_lsm6dso_init

Description :
    Probes the LSM6DSO by reading WHO_AM_I and configures basic ODR/range
    for accelerometer and gyroscope (12.5 Hz, ±2g and 250 dps).

Parameter :
    void

Return :
    int : 0 on success, -ENODEV if WHO_AM_I mismatches, or negative errno on I2C failure

Example Call :
    int ret = imu_lsm6dso_init();
*/
int imu_lsm6dso_init(void)
{
    uint8_t who_am_i = 0;
    int ret;

    if (!device_is_ready(i2c_dev))
    {
        LOG_ERR("I2C device %s is not ready!", i2c_dev->name);
        return -1;
    }
    LOG_INF("I2C device %s is ready.", i2c_dev->name);

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

/*
Function : imu_readDisplay_raw_data

Description :
    Reads raw accelerometer and gyroscope data from the LSM6DSO and logs it.
    This function is intended to be called periodically from a dedicated
    thread to demonstrate sensor operation.

Parameter :
    void

Return :
    void

Example Call :
    imu_readDisplay_raw_data();
*/
void imu_readDisplay_raw_data(void)
{

    struct lsm6dso_raw_data sensor_data;

    // Fetch raw data
    if (lsm6dso_fetch_raw_data(i2c_dev, &sensor_data) == 0)
    {
        // Display raw data
        lsm6dso_display_raw_data(&sensor_data);
    }
    else
    {
        LOG_ERR("Failed to fetch data.");
    }
}