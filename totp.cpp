/**
 * @file totp.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functionality for generating and validating TOTP.
 * @bug No known bugs.
 */
#include "totp.hpp"

/**
 * @brief does the hmac calculation
 */
int manual_HMAC(uint8_t *secret, uint8_t *counter, uint8_t *digest) {
  // H((secret xor ipad) + counter)
  uint8_t i_key[PSA_HASH_MAX_SIZE] = {0};
  for (int i = 0; i < 10; i++) {
    i_key[i] = secret[i] ^ 0x36;
  }
  for (int i = 10; i < PSA_HASH_MAX_SIZE; i++) {
    i_key[i] = 0x36;
  }

  uint8_t i_sha[SHA1_DIGEST_LENGTH];

  /* Compute hash of inner message  */
  mbedtls_sha1_context *ctx =
      (mbedtls_sha1_context *)malloc(sizeof(mbedtls_sha1_context));

  mbedtls_sha1_starts_ret(ctx);
  mbedtls_sha1_update_ret(ctx, i_key, SHA1_BLOCKSIZE);
  mbedtls_sha1_update_ret(ctx, counter, 8);
  mbedtls_sha1_finish_ret(ctx, i_sha);

  // HMAC = H[(secret xor opad) + H((secret xor ipad) + counter)];
  uint8_t o_key[PSA_HASH_MAX_SIZE] = {0};
  for (int i = 0; i < 10; i++) {
    o_key[i] = secret[i] ^ 0x5c;
  }
  for (int i = 10; i < PSA_HASH_MAX_SIZE; i++) {
    o_key[i] = 0x5c;
  }

  /* Compute hash of outer message  */
  mbedtls_sha1_starts_ret(ctx);
  mbedtls_sha1_update_ret(ctx, o_key, SHA1_BLOCKSIZE);
  mbedtls_sha1_update_ret(ctx, i_sha, SHA1_DIGEST_LENGTH);
  mbedtls_sha1_finish_ret(ctx, digest);

  return 0;
}

/**
 * @brief dynamic truncation
 */
int DT(uint8_t *hmac_result) {
  // take the lowest 4 bits of hmac
  uint8_t offset = hmac_result[19] & 0xf;
  // idk why they just labelled this one p in the docs
  int p = (hmac_result[offset] & 0x7f) << 24 |
          (hmac_result[offset + 1] & 0xff) << 16 |
          (hmac_result[offset + 2] & 0xff) << 8 |
          (hmac_result[offset + 3] & 0xff);

  return p;
}

int validate_for_time(const char *secret_hex, const char *TOTP_string,
                      time_t unix_time) {
  // t0 = 0, timestep = 30
  unsigned long counter = unix_time / 30;
  uint8_t *counter_bytes = (uint8_t *)malloc(8);
  for (int i = 7; i >= 0; i--) {
    counter_bytes[i] = counter;
    counter >>= 8;
  }

  // convert to binary (byte array)
  uint8_t *secret_bytes = (uint8_t *)malloc(10);
  int pos = 0;
  for (int i = 0; i < 10; i++) {
    sscanf(secret_hex + pos, "%02hhx", &secret_bytes[i]);
    pos += 2;
  }

  uint8_t *hmac_out = (uint8_t *)malloc(SHA1_DIGEST_LENGTH);

  manual_HMAC(secret_bytes, counter_bytes, hmac_out);
  int TOTP = DT(hmac_out) % 1000000;
  int TOTP_input = atoi(TOTP_string);

  return TOTP == TOTP_input;
}

int validate(const char *secret_hex, const char *TOTP_string) {
  time_t current_time = time(NULL);
  return validate_for_time(secret_hex, TOTP_string, current_time) ||
         validate_for_time(secret_hex, TOTP_string, current_time + 30) ||
         validate_for_time(secret_hex, TOTP_string, current_time - 30);
}
