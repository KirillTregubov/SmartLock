# SmartLock

Made by Kirill Tregubov and Philip Cai.

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

The devices manages a lock assigned to pin D7. To unlock, a valid time-based one-time password or a recovery key must be transmitted to the device over Bluetooth.

The private key and six recovery keys are generated on first boot and stored on the device. They are retreived from storage on subsequent system boots.

A QR code is generated and displayed on every device boot which can be scanned into an authenticator app using a mobile device to generate the time-based one-time passwords.

## Usage
1. Scan QR code using an authenticator application on a mobile device
2. Retrieve time-based one-time password from authenticator (TOTP).
3. Connect to 'SmartLock' using Bluetooth.
4. Write TOTP as a byte array or a UTF string to the writeable characteristic.
5. Alternatively, you can write one of the 6 recovery keys to the characteristic.

## Features

- Wireless Network RTC Synchronization
  - If provided network credentials, the device will automatically attempt to sync device time to that specified by a public NTP Server.
- Bluetooth Connectivity and Communication
- Press USER button to display device logs
- Hold USER button for 5 seconds to reset device
- Time-based One-time Password Validation
- One-time Recovery Key Matching
- QR code Generation and Display

## Troubleshooting Tips
If you are seeing issues with mbedtls_sha1, navigate to mbed-os/connectivity/mbedtls/include/mbedtls/config.h, and uncomment the macro #define MBEDTLS_SHA1_C

### Note
The board RTC time and your authenticator device must be synchronized to the current NTP time for validation to function. Verify that your device and the board RTC time have been synchronized before attempting unlock