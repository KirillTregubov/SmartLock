// Header file for totp validation.
#ifndef TOTP_H
#define TOTP_H

#include <mbed.h>

// Returns 1 if valid, 0 otherwise.
int validateTOTP(const char * secret_hex, const char * TOTP_string, time_t unix_time);

#endif