/* SmartLock
 * Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * Software is distributed and provided "AS IS", WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "TCPSocket.h"
#include "mbed.h"
#include "mbedtls/version.h" // unsure if needed
#include "ntp-client/NTPClient.h"
#include "psa/crypto.h"
#include <ctype.h>

// #define WIFI_IDW0XX1    2
#if (defined(TARGET_DISCO_L475VG_IOT01A))
#include "ISM43362Interface.h"
ISM43362Interface wifi(false);
#endif

const char *sec2str(nsapi_security_t sec) {
  switch (sec) {
  case NSAPI_SECURITY_NONE:
    return "None";
  case NSAPI_SECURITY_WEP:
    return "WEP";
  case NSAPI_SECURITY_WPA:
    return "WPA";
  case NSAPI_SECURITY_WPA2:
    return "WPA2";
  case NSAPI_SECURITY_WPA_WPA2:
    return "WPA/WPA2";
  case NSAPI_SECURITY_UNKNOWN:
  default:
    return "Unknown";
  }
}

int scan_demo(WiFiInterface *wifi) {
  WiFiAccessPoint *ap;

  printf("> Scan WiFi\n");

  return wifi->scan(NULL, 0);
}

void set_rtc() {
  NTPClient ntp(&wifi);
  // default server: 2.pool.ntp.org and default port: 123
  time_t timestamp = ntp.get_timestamp();
  if (timestamp < 0) {
    printf("An error occurred when getting the time. (code %u)\n", timestamp);
  } else {
    printf("> Set RTC to %s", ctime(&timestamp));
    set_time(timestamp);
  }
}

char *strupr(char *str) {
  unsigned char *p = (unsigned char *)str;

  // Convert to upper case
  while (*p) {
    *p = toupper((unsigned char)*p);
    p++;
  }

  return str;
}

void generate_private_key() {
  printf("> Generate key\n");

  psa_status_t status;
  status = psa_crypto_init();
  if (status != PSA_SUCCESS) {
    printf("Failed to initialize PSA Crypto\n");
    return;
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
    return;
  }

  status = psa_export_key(id, exported, sizeof(exported), &exported_length);
  if (status != PSA_SUCCESS || exported_length != 10) {
    printf("Failed to export key (%d)\n", status);
    return;
  }

  char key[exported_length];
  int index = 0;
  for (int i = 0; i < exported_length; i++) {
    index += sprintf(&key[index], "%02x", exported[i]);
  }
  printf("Key: %s (len: %d)\n", strupr(key), strlen(key));

  /* Clean up */
  psa_reset_key_attributes(&attributes);
  mbedtls_psa_crypto_free();
}

int main() {
  printf("=== SmartLock booted ===\n");
  generate_private_key();

  int count = scan_demo(&wifi);
  if (count == 0) {
    printf("No WiFi hotspots found - can't continue further.\n");
    return -1;
  }

  printf("> Connect to %s...\n", MBED_CONF_APP_WIFI_SSID);
  int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD,
                         NSAPI_SECURITY_WPA_WPA2);
  if (ret != 0) {
    printf("Connection error\n");
    return -1;
  }

  printf("> Connection success! IP: %s\n", wifi.get_ip_address());
  set_rtc();

  wifi.disconnect();
  printf("Terminated\n");
}
