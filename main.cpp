/* SmartLock
 * Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * Software is distributed and provided "AS IS", WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
 */
 
#include "mbed.h"
#include "TCPSocket.h"
 
#define WIFI_IDW0XX1    2
 
#if (defined(TARGET_DISCO_L475VG_IOT01A) || defined(TARGET_DISCO_F413ZH))
#include "ISM43362Interface.h"
//MBED_CONF_APP_WIFI_SPI_MOSI, MBED_CONF_APP_WIFI_SPI_MISO, MBED_CONF_APP_WIFI_SPI_SCLK, MBED_CONF_APP_WIFI_SPI_NSS, MBED_CONF_APP_WIFI_RESET, MBED_CONF_APP_WIFI_DATAREADY, MBED_CONF_APP_WIFI_WAKEUP, 
ISM43362Interface wifi(false);
#endif

#include "psa/crypto.h"
#include "mbedtls/version.h" // unsure if needed
#include "ntp-client/NTPClient.h"

const char *sec2str(nsapi_security_t sec)
{
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
 
int scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;
 
    printf("Scan:\n");
 
    int count = wifi->scan(NULL,0);
    printf("%d networks available.\n", count);
 
    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;
 
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
    for (int i = 0; i < count; i++)
    {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
 
    delete[] ap;
    return count;
}

void set_rtc() {
    NTPClient ntp(&wifi);
    // default server: 2.pool.ntp.org and default port: 123
    time_t timestamp = ntp.get_timestamp();
    if (timestamp < 0) {
        printf("An error occurred when getting the time. (code %u)\n", timestamp);
    } else {
        printf("Set RTC clock to %s\n", ctime(&timestamp));
        set_time(timestamp);
    }
}
void generate_private_key()
{
    uint8_t random[20] = { 0 };

    printf("Generating random...\n");

    psa_status_t status;
    status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        printf("Failed to initialize PSA Crypto\n");
        return;
    }

    status = psa_generate_random(random, sizeof(random));
    if (status != PSA_SUCCESS) {
        printf("Failed to generate a random value\n");
        return;
    }

    printf("Generated %s\n", random);

    /* Clean up */
    mbedtls_psa_crypto_free();
}

int main()
{
    generate_private_key();
    
    int count = scan_demo(&wifi);
    if (count == 0) {
        printf("No WIFI APNs found - can't continue further.\n");
        return -1;
    }
 
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
 
    printf("Success! Connection Info:\n");
    printf("MAC: %s\n", wifi.get_mac_address());
    printf("IP: %s\n", wifi.get_ip_address());
    printf("Netmask: %s\n", wifi.get_netmask());
    printf("Gateway: %s\n", wifi.get_gateway());
    printf("RSSI: %d\n\n", wifi.get_rssi());

    set_rtc();
 
    wifi.disconnect();
    printf("\nTerminated\n");
}
