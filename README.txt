To install required libraries before building, call:
$ mbed deploy
Once libraries are up to date, navigate to mbed-os/connectivity/mbedtls/include/mbedtls/config.h, and uncomment the macro #MBEDTLS_SHA1_C
