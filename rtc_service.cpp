/**
 * @file rtc_service.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functions for syncing the RTC.
 * @bug No known bugs.
 */
#include "rtc_service.hpp"

void sync_rtc_with_factory() {
  set_time(1648016868);
  time_t factory_time = time(NULL);
  printf("> Defaulted RTC to %s", ctime(&factory_time));
}

void sync_rtc_with_ntp(NetworkInterface *wifi) {
  NTPClient ntp(wifi);
  time_t timestamp = ntp.get_timestamp();
  if (timestamp < 0) {
    printf("An error occurred when getting the time. (code %u)\n", timestamp);
    sync_rtc_with_factory();
  } else {
    printf("> Synced RTC to %s", ctime(&timestamp));
    set_time(timestamp);
  }
}
