cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ThaneHunt_Project)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)

# Add the component app_ble
target_sources(app PRIVATE
    components/app_ble/app_ble.c)
target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/components/app_ble
)

# Add the component app_button
target_sources(app PRIVATE
    components/app_button/app_button.c)
target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/components/app_button
)

# Add the component app_hid
target_sources(app PRIVATE
    components/app_hid/app_hid.c)
target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/components/app_hid
)

# Add the component app_imu
target_sources(app PRIVATE
    components/app_imu/app_imu.c)
target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/components/app_imu
)
# Add the component app_keycodes
target_sources(app PRIVATE
    components/app_keycodes/app_keycodes.c)
target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/components/app_keycodes
)
