/*
Name : app_imu

Description : 
    LSM6DSO IMU interface for Zephyr RTOS over I2C. This module initializes
    the sensor, periodically reads raw accelerometer/gyroscope samples in a
    dedicated thread, and logs the raw LSB values for debugging/bring-up.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

#ifndef APP_IMU_H
#define APP_IMU_H

int imu_lsm6dso_init(void);
void imu_readDisplay_raw_data(void);
int lsm6dso_accel_gyro_power_down(void);

#endif // APP_IMU_H
