/**
 * @file rtc_service.hpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functions for syncing the RTC.
 * @bug No known bugs.
 */
#ifndef RTC_SERVICE_H
#define RTC_SERVICE_H

#include "mbed.h"
#include "ntp-client/NTPClient.h"

/**
 * @brief Synchronizes the RTC to a hardcoded epoch.
 *
 * @return Void.
 */
void sync_rtc_with_factory();

/**
 * @brief Synchronizes the RTC to the epoch returned by
 *        the default NTP server.
 *
 * The default NTP server is 2.pool.ntp.org port 123.
 *
 * @return Void.
 *
 * Precondition: wifi connection has been established.
 */
void sync_rtc_with_ntp(NetworkInterface *wifi);

#endif // RTC_SERVICE_H
