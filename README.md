# SmartLock

Made by [Kirill Tregubov](https://github.com/KirillTregubov) and [Philip Cai](https://github.com/Gadnalf).

## Getting Started

You must have [Mbed Studio](https://os.mbed.com/studio/) installed and be logged into an account to build and run this project.

This project was built and designed for the [DISCO-L475VG-IOT01A](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/) board.

To install the required libraries before building, run the following command:
```
mbed deploy
```

### Note
Before building the project, it is important that you edit `wifi_credentials.hpp` and input correct WiFi credentials for your setup.

## Functionality

The devices manages a lock assigned to pin D7. To unlock, a valid time-based one-time password (TOTP) or a recovery key must be transmitted to the device over Bluetooth.

The private key and six recovery keys are generated on first boot and stored on the device. They are retreived from storage on subsequent system boots.

A QR code is generated and displayed on every device boot which can be scanned into an authenticator app using a mobile device to generate the time-based one-time passwords.

## Usage
1. Scan QR code using an authenticator application on a mobile device.
2. Retrieve TOTP from authenticator.
3. Connect to 'SmartLock' using Bluetooth.
4. Write TOTP as a byte array or a UTF string to the writeable characteristic.
5. Alternatively, you can write one of the 6 recovery keys once.

## Features

- Press USER Button to Display Device Logs
- Hold USER Button for 5 Seconds to Reset Lock (resets to locked)
- Wireless Network RTC Synchronization
  - If provided network credentials, the device will automatically attempt to sync device time to 4 different public NTP servers.
- Bluetooth Connectivity and Communication
- Device Key and Log Storage
- TOTP Submission and Validation
- One-time Recovery Key Usage
- QR code Generation and Display
- LEDs have Unique Blink Pattern on both Lock and Unlock
- Pin D7 has Rising Edge for Duration of Unlock

## Troubleshooting Tips
If you are seeing issues with mbedtls_sha1, navigate to `mbed-os/connectivity/mbedtls/include/mbedtls/config.h`, and uncomment the macro `#define MBEDTLS_SHA1_C`.

### Note
The board RTC time and your authenticator device must be synchronized to the current NTP time for validation to function. Verify that your device and the board RTC time have been synchronized before attempting unlock
