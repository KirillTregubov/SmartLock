/**
 * @file main.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 * @brief Main SmartLock program.
 *
 * This generates the private key, syncs the RTC with an NTP
 * server, and initializes the BLE.
 *
 * @bug No known bugs.
 */

#include "mbed.h"
#include "ntp-client/NTPClient.h"
#include <ctype.h>
// #include "TCPSocket.h"
// #include <time.h>
// #include "mbedtls/version.h"
// #include "psa/crypto.h"

#if defined(TARGET_DISCO_L475VG_IOT01A)
#include "ISM43362Interface.h"
ISM43362Interface wifi(false);
#endif

/**
 * @brief Synchronizes the RTC to a hardcoded epoch.
 *
 * @return Void.
 */
void sync_rtc_with_factory() {
  set_time(1648016868);
  time_t factory_time = time(NULL);
  printf("> Defaulted RTC to %s.", ctime(&factory_time));
}

/**
 * @brief Synchronizes the RTC to the epoch returned by
 *        the default NTP server.
 *
 * The default NTP server is 2.pool.ntp.org port 123.
 *
 * @return Void.
 */
void sync_rtc_with_ntp() {
  NTPClient ntp(&wifi);
  time_t timestamp = ntp.get_timestamp();
  if (timestamp < 0) {
    printf("An error occurred when getting the time. (code %u)\n", timestamp);
    sync_rtc_with_factory();
  } else {
    printf("> Synced RTC to %s", ctime(&timestamp));
    set_time(timestamp);
  }
}

/**
 * @brief Convert a string to uppercase.
 *
 * @param str The string that you want to convert to uppercase.
 * @return The address of the string.
 */
char *strupr(char *str) {
  unsigned char *temp = (unsigned char *)str;

  // Convert to upper case
  while (*temp) {
    *temp = toupper((unsigned char)*temp);
    temp++;
  }

  return str;
}

/**
 * @brief Generate a private key using onboard TRNG chip.
 *
 * @return The address of the generated key.
 */
char *generate_private_key() {
  printf("> Generate key\n");

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
  printf("Key: %s (len: %d)\n", strupr(key), strlen(key));

  psa_reset_key_attributes(&attributes);
  mbedtls_psa_crypto_free();

  return strupr(key);
}

/**
 * @brief Connect to the WiFi network specified in 'mbed_app.json'.
 *
 * @return 0 upon success, -1 on error / failure.
 */
int connect_to_wifi() {
  WiFiAccessPoint *ap;
  printf("> Searching for WiFi networks\n");
  int network_count = wifi.scan(NULL, 0);
  if (network_count <= 0) {
    printf("No WiFi hotspots found - can't continue further.\n");
    return -1;
  }

  printf("> Connecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
  int status = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD,
                         NSAPI_SECURITY_WPA_WPA2);
  if (status != 0) {
    printf("Connection error\n");
    return -1;
  }

  printf("> Connection success! IP: %s\n", wifi.get_ip_address());
  return 0;
}

int main() {
  printf("=== SmartLock booted ===\n");
  char *key = generate_private_key();
  printf("Generated key %s", key);

  int status = connect_to_wifi();
  if (status < 0) {
    sync_rtc_with_factory();
  } else {
    sync_rtc_with_ntp();
  }
  wifi.disconnect();


  printf("Terminated\n");
}
