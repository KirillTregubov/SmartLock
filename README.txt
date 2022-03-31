To install required libraries before building, call:
$ mbed deploy

If seeing issues with mbedtls_sha1, navigate to mbed-os/connectivity/mbedtls/include/mbedtls/config.h, and uncomment the macro #define MBEDTLS_SHA1_C
