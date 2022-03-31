/**
 * @file wifi_service.cpp
 * @author Kirill Tregubov (KirillTregubov), Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functions for connecting to the WiFi.
 * @bug No known bugs.
 */
#include "smartlock.hpp"

int connect_to_wifi(WiFiInterface *wifi) {
  WiFiAccessPoint *ap;
  printf("> Searching for WiFi networks\n");
  int network_count = wifi->scan(NULL, 0);
  if (network_count <= 0) {
    printf("No WiFi hotspots found - can't continue further.\n");
    return -1;
  }

  printf("> Connecting to %s...\n", WIFI_SSID);
  int status =
      wifi->connect(WIFI_SSID, WIFI_PASSWORD,
                    NSAPI_SECURITY_WPA_WPA2);
  if (status != 0) {
    printf("Connection error\n");
    return -1;
  }

  SocketAddress a;
  wifi->get_ip_address(&a);
  printf("> Connection success! IP: %s\n", a.get_ip_address());
  return 0;
}
