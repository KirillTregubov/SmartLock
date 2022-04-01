/**
 * @file wifi_service.hpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This header defines functionality for connecting to WiFi.
 * @bug No known bugs.
 */
#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include "mbed.h"

/**
 * @brief Connect to the WiFi network specified in 'mbed_app.json'.
 *
 * @param wifi A WiFiInterface instance.
 * @return 0 upon success, -1 on error / failure.
 */
int connect_to_wifi(WiFiInterface *wifi);

#endif // WIFI_SERVICE_H
