/**
 * @file totp.hpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This header file exports TOTP validation.
 * @bug No known bugs.
 */
#ifndef TOTP_H
#define TOTP_H

#include <assert.h>
#include <cstdint>
#include <mbed.h>
#include <ppp_opts.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHA1_DIGEST_LENGTH 20
#define SHA1_BLOCKSIZE 64

/**
 * @brief Validates a single TOTP value for a given secret at the device's current RTC time +-30.
 *
 * @param secret_hex The private secret as a hex string.
 * @param TOTP_string The input TOTP value as a string.
 * @return 1 if valid, 0 otherwise.
 */
int validate(const char *secret_hex, const char *TOTP_string);

#endif // TOTP_H
