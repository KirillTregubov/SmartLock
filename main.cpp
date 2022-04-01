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

#include "mbed.h"
#include "smartlock.hpp"
#include "datastore.hpp"
#include "qrcodegen.hpp"
#include "totp.hpp"

using qrcodegen::QrCode;

#if defined(TARGET_DISCO_L475VG_IOT01A)
#include "ISM43362Interface.h"
ISM43362Interface wifi(false);
#endif

static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);
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

  char key[exported_length];
  int index = 0;
  for (int i = 0; i < exported_length; i++) {
    index += sprintf(&key[index], "%02x", exported[i]);
  }
  // printf("Key: %s (len: %d)\n", strupr(key), strlen(key));

  psa_reset_key_attributes(&attributes);
  mbedtls_psa_crypto_free();

  strncpy(buffer, strupr(key), buffersize);
  return 0;
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
  printf("\n");
}

int main() {
  printf("=== SmartLock booted ===\n");

  char key[20];
  generate_private_key(key, sizeof(key));
  printf("> Generated key is %s\n", key);

  // TODO: generate base32 secret
  const QrCode qr0 = QrCode::encodeText(
      "otpauth://totp/SmartLock?secret=JBSWY3DPEHPK3PXP", QrCode::Ecc::MEDIUM);
  printQr(qr0);

  int status = connect_to_wifi(&wifi);
  if (status < 0) {
    sync_rtc_with_factory();
  } else {
    sync_rtc_with_ntp(&wifi);
  }
  wifi.disconnect();

  printf("> Mounting file system\n");
  mount_fs();

  printf("> Setting up log output\n");
  button1.fall(event_queue.event(print_logs));

  SmartLock smart_lock(&event_queue);

  printf("> Starting BLE server\n");
  init_bluetooth(event_queue, &smart_lock);

  printf("> Terminated\n");
}
