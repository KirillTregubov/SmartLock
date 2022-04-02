/**
 * @file main.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief Main SmartLock program.
 * This module generates the private key, prints an Authenticator QR code, syncs
 * the RTC with an NTP server, mounts the filesystem, registers the log output,
 * and initializes the BLE.
 *
 * @bug No known bugs.
 */
#include "ble_service.hpp"
#include "datastore.hpp"
#include "helpers.hpp"
#include "mbed.h"
#include "qrcodegen.hpp"
#include "rtc_service.hpp"
#include "smartlock.hpp"
#include "wifi_service.hpp"
#include <cstdint>
#include <cstdio>


using qrcodegen::QrCode;

#if defined(TARGET_DISCO_L475VG_IOT01A)
#include "ISM43362Interface.h"
ISM43362Interface wifi(false);
#endif

// static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);
EventQueue event_queue;
InterruptIn button1(BUTTON1);

/**
 * @brief Generate a private key using onboard TRNG chip.
 *
 * @return The address of the generated key.
 */
int generate_private_key(char *buffer, int buffersize) {
  psa_status_t status;
  status = psa_crypto_init();
  if (status != PSA_SUCCESS) {
    printf("Failed to initialize PSA Crypto\n");
    return NULL;
  }

  size_t exported_length = 0;
  static uint8_t exported[10];
  psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
  psa_key_id_t id;
  psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT);
  psa_set_key_algorithm(&attributes,
                        PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256));
  psa_set_key_type(&attributes, PSA_KEY_TYPE_RAW_DATA);
  psa_set_key_bits(&attributes, 80);

  status = psa_generate_key(&attributes, &id);
  if (status != PSA_SUCCESS) {
    printf("Failed to generate key (%d)\n", status);
    return NULL;
  }

  status = psa_export_key(id, exported, sizeof(exported), &exported_length);
  if (status != PSA_SUCCESS || exported_length != 10) {
    printf("Failed to export key (%d)\n", status);
    return NULL;
  }

  char key[21] = {0};
  int index = 0;
  for (int i = 0; i < 10; i++) {
    index += sprintf(&key[index], "%02x", exported[i]);
  }
  key[index] = '\0';

  psa_reset_key_attributes(&attributes);
  mbedtls_psa_crypto_free();

  strncpy(buffer, strupr(key), buffersize);
  return 0;
}

void generate_reset() {
  psa_status_t status;
  uint8_t random[36] = {0};

  fflush(stdout);

  status = psa_crypto_init();
  if (status != PSA_SUCCESS) {
    printf("Failed to initialize PSA Crypto\n");
    return;
  }

  status = psa_generate_random(random, sizeof(random));
  if (status != PSA_SUCCESS) {
    printf("Failed to generate a random value (%" PRIu32 ")\n", status);
    return;
  }

  char keys[6][7];
  for (int i = 0; i < 36; i++) {
    keys[i / 6][i % 6] = 'A' + (random[i] % 26);
  }

  printf("> Generated recovery keys\n");

  for (int i = 0; i < 6; i++) {
    keys[i][6] = '\0';
    printf("%s\n", keys[i]);
  }

  mbedtls_psa_crypto_free();
}

/**
 * @brief Prints the given QrCode object to the console.
 *
 * From the QR Code generator library (C++)
 * https://www.nayuki.io/page/qr-code-generator-library
 *
 * @author Project Nayuki (nayuki)
 * @copyright Copyright (c) Project Nayuki. (MIT License)
 */
void printQr(const QrCode &qr) {
  int border = 2;
  for (int y = -border; y < qr.getSize() + border; y++) {
    for (int x = -border; x < qr.getSize() + border; x++) {
      printf(qr.getModule(x, y) ? "⬛⬛" : "⬜⬜");
    }
    printf("\n");
  }
}

int hex_to_base32(const char *hex, int length, char *result, int bufSize) {
  uint8_t *bytes = (uint8_t *)malloc(10);
  int pos = 0;
  for (int i = 0; i < 10; i++) {
    sscanf(hex + pos, "%2hhx", &bytes[i]);
    pos += 2;
  }

  length = length / 2;

  if (length < 0 || length > (1 << 28)) {
    return -1;
  }
  int count = 0;
  if (length > 0) {
    int buffer = bytes[0];
    int next = 1;
    int bitsLeft = 8;
    while (count < bufSize && (bitsLeft > 0 || next < length)) {
      if (bitsLeft < 5) {
        if (next < length) {
          buffer <<= 8;
          buffer |= bytes[next++] & 0xFF;
          bitsLeft += 8;
        } else {
          int pad = 5 - bitsLeft;
          buffer <<= pad;
          bitsLeft += pad;
        }
      }
      int index = 0x1F & (buffer >> (bitsLeft - 5));
      bitsLeft -= 5;
      result[count++] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"[index];
    }
  }
  if (count < bufSize) {
    result[count] = '\000';
  }
  return count;
}

int main() {
  printf("=== SmartLock booted ===\n");

  SmartLock smart_lock(&event_queue);

  generate_reset();

  printf("> Mounting file system\n");
  mount_fs();
  write_log("Device booted");

  char key[20];
  generate_private_key(key, 20);
  printf("> Generated key is %s (len: %d)\n", key, strlen(key));
  set_private_key(key);

  char base32key[20];
  hex_to_base32(key, 20, base32key, 20);

  char qr_uri[52];
  sprintf(qr_uri, "otpauth://totp/SmartLock?secret=%s", base32key);
  printf("%s\n", qr_uri);

  printf("> Scan the following code using an authenticator app on your mobile "
         "device\n");
  const QrCode qr0 = QrCode::encodeText(qr_uri, QrCode::Ecc::MEDIUM);
  printQr(qr0);

  int status = connect_to_wifi(&wifi);
  if (status < 0) {
    sync_rtc_with_factory();
  } else {
    sync_rtc_with_ntp(&wifi);
  }
  wifi.disconnect();

  printf("> Setting up log output\n");
  button1.fall(event_queue.event(print_logs));

  printf("> Initializing BLE broadcast\n");
  init_bluetooth(event_queue, &smart_lock);

  printf("> Terminated\n");
}
