# ThaneHunt — BLE HID Keyboard (nRF54L15 + Zephyr)

A small, battery-friendly **BLE HID keyboard** firmware that advertises as `ThaneHunt_BLE_HID_KEYBOARD`, bonds securely, reports battery level, and can read raw data from an **LSM6DSO** IMU over I²C. It idles and drops into **deep sleep** when inactive, and can wake on a button (P1.0).

This README is written to be copy-paste ready for `README.md`. It covers the project layout, how to build/flash, what the logs mean, and—most importantly—**every project Kconfig you can toggle (what it does and how to enable/disable it).**

---

## TL;DR (What it does)

* **BLE HID Keyboard (HIDS)** peripheral that pairs & bonds, including optional **passkey auth**.
* **Battery Service (BAS)** notifications.
* **IMU (LSM6DSO)** bring-up + periodic raw accel/gyro readout via I²C (TWIM20 on P1.8/P1.9 by default).
* **Power management**: idle timeout → IMU power-down → BLE disconnect → **deep sleep**. Wake on **Button P1.0**.
* Clean, modular components under `components/`:

  * `app_ble/`, `app_hid/`, `app_button/`, `app_imu/`, `app_sleep/`, `app_keycodes/`.

Boards tested via `sample.yaml`:

* `xiao/nrf54l15/nrf54l15/cpuapp`
* `nrf54l15dk/nrf54l15/cpuapp`
* `panb511evb/nrf54l15/cpuapp`

> BLE name: **`ThaneHunt_BLE_HID_KEYBOARD`**
> Appearance: **Keyboard (0x03C1 / 961)**

---

## Example logs (successful boot & flow)

```
*** Booting nRF Connect SDK v3.0.2-89ba1294ac9b ***
*** Using Zephyr OS v4.0.99-f791c49f492c ***
[00:08:29.550,716] <inf> APP_BUTTON: Button (P1.0) was the wakeup source
[00:08:29.550,742] <inf> MAIN: Starting BLE HIDS keyboard VERSION: [1.0.0]

[00:08:29.551,613] <inf> fs_zms: 2 Sectors of 4096 bytes
[00:08:29.551,624] <inf> fs_zms: alloc wra: 0, bc0
[00:08:29.551,631] <inf> fs_zms: data wra: 0, 230
[00:08:29.552,093] <inf> bt_sdc_hci_driver: SoftDevice Controller build revision: 
                                            89 9a 50 8a 95 01 9c 58  fc 39 d2 c1 10 04 ee 02 |..P....X .9......
                                            64 ce 25 be                                      |d.%.             
[00:08:29.553,280] <inf> bt_hci_core: HW Platform: Nordic Semiconductor (0x0002)
[00:08:29.553,295] <inf> bt_hci_core: HW Variant: nRF54Lx (0x0005)
[00:08:29.553,316] <inf> bt_hci_core: Firmware: Standard Bluetooth controller (0x00) Version 137.20634 Build 2617349514
[00:08:29.553,441] <inf> bt_hci_core: No ID address. App must call settings_load()
[00:08:29.553,447] <inf> APP_BLE: Bluetooth initialized

[00:08:29.555,009] <inf> bt_hci_core: Identity: E6:09:BB:6C:F1:6B (random)
[00:08:29.555,026] <inf> bt_hci_core: HCI: version 6.0 (0x0e) revision 0x30f3, manufacturer 0x0059
[00:08:29.555,040] <inf> bt_hci_core: LMP: version 6.0 (0x0e) subver 0x30f3
[00:08:29.556,706] <inf> APP_BLE: Advertising successfully started

[00:08:29.556,727] <inf> APP_IMU: I2C device i2c@104000 is ready.
[00:08:29.556,915] <inf> APP_IMU: LSM6DSO WHO_AM_I check passed. ID: 0x6a
[00:08:29.557,164] <inf> APP_IMU: LSM6DSO initialized successfully.
[00:08:29.557,746] <inf> APP_IMU: accel raw: X:-138 Y:-94 Z:16829 (LSB)
[00:08:29.557,753] <inf> APP_IMU: gyro raw: X:214 Y:-484 Z:-104 (LSB)
[00:08:29.557,758] <inf> APP_IMU: trig_cnt:1
[00:08:36.996,380] <inf> bas: BAS Notifications enabled
[00:08:36.996,630] <inf> APP_BLE: Connected 04:EA:56:AE:71:66 (public)

[00:08:37.562,779] <inf> APP_BUTTON: User LED Turn OFF

[00:08:37.563,369] <inf> APP_IMU: accel raw: X:15894 Y:910 Z:3441 (LSB)
[00:08:37.563,377] <inf> APP_IMU: gyro raw: X:239 Y:-448 Z:-92 (LSB)
[00:08:37.563,382] <inf> APP_IMU: trig_cnt:9

[00:08:38.564,191] <inf> APP_IMU: accel raw: X:15902 Y:912 Z:3435 (LSB)
[00:08:38.564,204] <inf> APP_IMU: gyro raw: X:258 Y:-430 Z:-87 (LSB)
[00:08:38.564,209] <inf> APP_IMU: trig_cnt:10

[00:08:38.983,902] <inf> APP_BLE: Pairing completed: 04:EA:56:AE:71:66 (public), bonded: 1
[00:08:38.985,237] <inf> APP_BLE: Security changed: 04:EA:56:AE:71:66 (public) level 4
[00:08:39.058,397] <inf> APP_HID: Cannot clear selected key.

[00:09:08.589,664] <inf> APP_IMU: accel raw: X:15887 Y:955 Z:3441 (LSB)
[00:09:08.589,672] <inf> APP_IMU: gyro raw: X:240 Y:-459 Z:-88 (LSB)
[00:09:08.589,677] <inf> APP_IMU: trig_cnt:40

[00:09:09.059,157] <inf> APP_SLEEP: LSM6DSO powered down
[00:09:09.179,365] <wrn> APP_SLEEP: No activity -> disconnect + deep sleep
```

---

## Project structure

```
ThaneHunt_Project/
├─ CMakeLists.txt
├─ Kconfig
├─ Kconfig.sysbuild
├─ prj.conf
├─ sample.yaml
├─ boards/
│  ├─ xiao_nrf54l15_nrf54l15_cpuapp.overlay
│  ├─ nrf54l15dk_nrf54l15_cpuapp.overlay
│  └─ panb511evb_nrf54l15_cpuapp.overlay
├─ src/
│  └─ main.c
└─ components/
   ├─ app_ble/      # GAP/GATT, pairing, advertising, BAS/HIDS plumbing
   ├─ app_hid/      # HID report map, key handling
   ├─ app_button/   # wake button + LED handling
   ├─ app_imu/      # LSM6DSO driver wrapper + raw reads
   ├─ app_sleep/    # idle timer → power-down → deep sleep
   └─ app_keycodes/ # HID keycode helpers
```

---

## Requirements

* **nRF Connect SDK**: 3.0.x
* **Zephyr**: 4.0.x (brought by NCS)
* **Targets**: nRF54L15 (DK, XIAO, PanB511 EVB) — see `sample.yaml`
* **Tooling**: `west`, `cmake`, `ninja` (installed via nRF Connect Toolchain Manager or CLI install)

---

## Build & flash

```bash
# 1) Get an nRF Connect SDK v3.0 workspace (or later 3.0.x)
# 2) From your nRF workspace:
west init -m https://github.com/NordicPlayground/nrf --mr v3.0.0   # example
west update
west zephyr-export

# 3) Build (pick your board)
cd ThaneHunt_Project
west build -b nrf54l15dk/nrf54l15/cpuapp .

# or for XIAO nRF54L15:
west build -b xiao/nrf54l15/nrf54l15/cpuapp .

# 4) Flash
west flash
# 5) View logs
west espressif monitor   # or `west attach` / your preferred UART monitor
```

> This sample uses **sysbuild** with `NRF_DEFAULT_IPC_RADIO=y` (see `Kconfig.sysbuild`), so the radio core image is managed for you.

---

## Device tree (pins, I²C)

The board overlays configure **TWIM20** on **P1.8 (SCL)** and **P1.9 (SDA)** and expose `i2c20`.
`app_imu` probes the LSM6DSO and prints WHO\_AM\_I = `0x6A` on success.

Wake source is **Button (P1.0)**; you’ll see that explicitly in the boot log.

---

## HID behavior

* HIDS is enabled with encryption-required permissions.
* Max clients: `CONFIG_BT_HIDS_MAX_CLIENT_COUNT=1` (changeable).
* The report map and keyboard events live under `components/app_hid/`.
* Battery Service notifications are sent periodically.

---

## Power & sleep

`components/app_sleep/` starts an **idle timer**. After `CONFIG_DEVICE_IDLE_TIMEOUT_SECONDS` of no activity:

1. **IMU power-down** (you’ll see “LSM6DSO powered down”).
2. Optional **disconnect**, then system enters **deep sleep**.
3. **Button P1.0** wakes the device.

---

## Security

* **Bonding** + L2 security upgrade to **Level 4** (LE Secure Connections + encryption).
* Optional **passkey authentication**:

  * Project toggle: `CONFIG_ENABLE_PASS_KEY_AUTH`
  * Uses Zephyr’s `CONFIG_BT_FIXED_PASSKEY` (set a fixed PIN) if desired.

---

## The important part — Kconfig & `prj.conf` options (what they do & how to toggle)

You can enable/disable features either by editing `prj.conf` directly or via menuconfig:

```bash
west build -b <your_board> -t menuconfig
# or for sysbuild:
west build -b <your_board> -t guiconfig
```

> When editing `prj.conf`, `y` = enabled, `n` = disabled, strings in quotes, and integers as numbers.

### Project-specific Kconfig (defined in this repo’s `Kconfig`)

| Symbol                                                  | Type     |                  Default | What it does                                                                                                                                           | How to use                                                                                      |
| ------------------------------------------------------- | -------- | -----------------------: | ------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------- |
| `CONFIG_PROJECT_VERSION`                                | `string` |                `"0.0.0"` | Version string shown in logs (e.g. `Starting BLE HIDS keyboard VERSION: [1.0.0]`) and can be exposed in DIS if you wire it.                            | Set to your firmware version, e.g. `CONFIG_PROJECT_VERSION="1.2.3"`.                            |
| `CONFIG_ENABLE_PASS_KEY_AUTH`                           | `bool`   |                      `n` | Enables additional passkey authentication flow in the BLE layer. Works with Zephyr’s `CONFIG_BT_FIXED_PASSKEY`.                                        | Set `CONFIG_ENABLE_PASS_KEY_AUTH=y` and also `CONFIG_BT_FIXED_PASSKEY=y` + passkey (see below). |
| `CONFIG_NFC_OOB_PAIRING` *(implied by Kconfig selects)* | `bool`   |                      `n` | Enables NFC NDEF / LE OOB pairing path. When `y`, the Kconfig selects turn on `NFC_NDEF`, `NFC_NDEF_MSG`, `NFC_NDEF_RECORD`, `NFC_NDEF_LE_OOB_REC`.    | Turn `CONFIG_NFC_OOB_PAIRING=y` to experiment with NFC OOB (requires HW).                       |
| `CONFIG_SETTINGS`                                       | `bool`   |                      `y` | Loads Zephyr **settings** backend so Bluetooth can retrieve identity/bonds on boot. Explains the “App must call settings\_load()” message if disabled. | Keep `y` unless you know what you’re doing.                                                     |
| `CONFIG_ZMS`                                            | `bool`   | `y` on nRF **RRAM/MRAM** | Selects **ZMS** (Zephyr memory storage) as the persistent storage backend when the SoC has RRAM/MRAM.                                                  | Leave default.                                                                                  |
| `CONFIG_NVS`                                            | `bool`   |   `y` when not RRAM/MRAM | Selects **NVS** flash storage backend on platforms without RRAM/MRAM.                                                                                  | Leave default.                                                                                  |

> The `Kconfig` file also wires the NFC selections (as above) when NFC OOB is turned on.

### Project toggles located in `prj.conf` (and used in code)

| Symbol                                                                                | What it does                                                                         | Typical value                                        |
| ------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ | ---------------------------------------------------- |
| `CONFIG_BT=y`                                                                         | Turns on Zephyr Bluetooth stack.                                                     | **Required**.                                        |
| `CONFIG_BT_PERIPHERAL=y`                                                              | Operate as a BLE peripheral (advertises, accepts connections).                       | **Required**.                                        |
| `CONFIG_BT_DEVICE_NAME="ThaneHunt_BLE_HID_KEYBOARD"`                                  | GAP name.                                                                            | Change if you want a different name.                 |
| `CONFIG_BT_DEVICE_APPEARANCE=961`                                                     | GAP appearance (961 = Keyboard).                                                     | Leave unless changing device class.                  |
| `CONFIG_BT_BAS=y`                                                                     | Enable **Battery Service**.                                                          | `y`                                                  |
| `CONFIG_BT_HIDS=y`                                                                    | Enable **HID over GATT**.                                                            | `y`                                                  |
| `CONFIG_BT_HIDS_MAX_CLIENT_COUNT=1`                                                   | Max concurrent HIDS clients.                                                         | `1` (raise if multi-host needed).                    |
| `CONFIG_BT_HIDS_DEFAULT_PERM_RW_ENCRYPT=y`                                            | Require encryption for HIDS attrs.                                                   | `y` (security).                                      |
| `CONFIG_BT_SMP=y`                                                                     | Security Manager Protocol (pairing/bonding).                                         | `y`                                                  |
| `CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE=y`                                              | Allow rebonding from an unauth peer to overwrite existing bond (useful for dev).     | Optional.                                            |
| `CONFIG_BT_ID_UNPAIR_MATCHING_BONDS=y`                                                | Unpair matching bonds on rebond.                                                     | Optional.                                            |
| `CONFIG_BT_FIXED_PASSKEY=y`                                                           | Use a fixed passkey (works with `CONFIG_ENABLE_PASS_KEY_AUTH`).                      | `y` + set the passkey in code/Kconfig if needed.     |
| `CONFIG_ENABLE_PASS_KEY_AUTH=y`                                                       | **Project switch**: enable passkey flow in app layer.                                | `y` to enforce passkey pairing.                      |
| `CONFIG_PROJECT_VERSION="1.0.0"`                                                      | Version string used by app logs.                                                     | Adjust per release.                                  |
| `CONFIG_DEVICE_IDLE_TIMEOUT_SECONDS=30`                                               | **Project switch**: seconds of inactivity before power actions. Used by `app_sleep`. | Tune for your product.                               |
| `CONFIG_IMU_LSM6DSO=y`                                                                | **Project switch**: include IMU module & read raw accel/gyro; power down on sleep.   | `y` to enable IMU path; set `n` to strip it.         |
| `CONFIG_LSM6DS0=n`                                                                    | Ensure the older LSM6DS0 driver isn’t pulled in by mistake.                          | Keep `n`.                                            |
| `CONFIG_POWEROFF=y`                                                                   | Enables system power-off API (deep sleep).                                           | `y`                                                  |
| `CONFIG_HWINFO=y`                                                                     | Enables hardware info API (used for IDs, etc.).                                      | `y`                                                  |
| `CONFIG_NEWLIB_LIBC=y` + `CONFIG_NEWLIB_LIBC_FLOAT_PRINTF=y`                          | Newlib C and float printf.                                                           | Optional but handy for logs.                         |
| `CONFIG_UART_ASYNC_API`, `CONFIG_UART_NRFX_UARTE_ENHANCED_RX`, `CONFIG_UART_20_ASYNC` | Async UART for logging/console on nRF54L15.                                          | Provided by board/Kconfig; leave unless customizing. |

> There are additional BT buffer sizing/tuning options present (ACL sizes, RX/TX counts). Defaults here are conservative and suitable for a single-host keyboard.

#### How to **enable/disable** functionality (examples)

* **Turn off IMU completely** (strip code, save power/flash):

  ```ini
  # prj.conf
  CONFIG_IMU_LSM6DSO=n
  ```
* **Disable passkey authentication** (pair without passkey):

  ```ini
  CONFIG_ENABLE_PASS_KEY_AUTH=n
  # and optionally
  CONFIG_BT_FIXED_PASSKEY=n
  ```
* **Change idle timeout to 5 minutes**:

  ```ini
  CONFIG_DEVICE_IDLE_TIMEOUT_SECONDS=300
  ```
* **Let multiple hosts connect** (not typical for a keyboard; you’ll also need report-routing logic):

  ```ini
  CONFIG_BT_HIDS_MAX_CLIENT_COUNT=2
  CONFIG_BT_MAX_CONN=2
  CONFIG_BT_MAX_PAIRED=2  # remember bonding limits
  ```

---

## Code hotspots (where the configs are used)

* `src/main.c`
  Prints version (`CONFIG_PROJECT_VERSION`), starts BLE/IMU loops.
* `components/app_ble/`
  Advertising, connection callbacks, pairing/bonding, security level upgrades. Passkey path is compiled when `CONFIG_ENABLE_PASS_KEY_AUTH` (and `CONFIG_BT_FIXED_PASSKEY`) are set.
* `components/app_hid/`
  HID report map and key event helpers.
* `components/app_imu/`
  LSM6DSO init + raw reads; compiled when `CONFIG_IMU_LSM6DSO=y`.
* `components/app_sleep/`
  Uses `CONFIG_DEVICE_IDLE_TIMEOUT_SECONDS` to decide when to power down IMU, disconnect, and enter deep sleep.
* `components/app_button/`
  Wake button (P1.0) + simple LED feedback.

---

## BLE\_HID\_KEYBOARD quick test

1. Build & flash.
2. On your phone/PC, scan and connect to **`ThaneHunt_BLE_HID_KEYBOARD`**.
3. Pair/bond. If passkey is **enabled**, you’ll be prompted for it (fixed passkey path).
4. You should see **Battery Service** and **HID** in your BLE explorer, and logs similar to the snippet above.
5. Leave it idle for `CONFIG_DEVICE_IDLE_TIMEOUT_SECONDS` to watch the power-down → deep sleep flow.

---

## Tips & common tweaks

* **Rename device**: change `CONFIG_BT_DEVICE_NAME` in `prj.conf`.
* **Appearance code**: `961` is “Keyboard”; set to something else if you change device type.
* **Storage backend**: Kconfig auto-selects **ZMS** (RRAM/MRAM) or **NVS** (Flash) depending on SoC; keep defaults.
* **NFC OOB pairing** *(optional)*: turn on `CONFIG_NFC_OOB_PAIRING=y` and ensure the board supports NFC.

---

## License

Add your license text here (SPDX header in sources is recommended).

---

## Changelog

* **1.0.0** — Initial public release: BLE HID keyboard, BAS, IMU raw streaming (LSM6DSO), deep sleep, passkey option.

---

### Appendix: How to find and toggle configs

* **Menuconfig** shows all project and Zephyr symbols:

  ```
  west build -b <board> -t menuconfig
  ```

  * Project options are under a menu titled **“ThaneHunt BLE HID KEYBOARD”**.
  * Save to write back changes into `build/zephyr/.config` (mirror stable choices into `prj.conf`).

* **Direct edits**: put persistent choices in `prj.conf` (checked into VCS).

* **Sysbuild**: radio IPC side is enabled by `Kconfig.sysbuild`:

  ```kconfig
  # Kconfig.sysbuild
  source "share/sysbuild/Kconfig"
  config NRF_DEFAULT_IPC_RADIO
      default y
  ```