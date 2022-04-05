/**
 * @file ble_service.hpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This header defines functionality for initializing and using BLE.
 * @bug No known bugs.
 */
#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "BLE.h"
#include "Gap.h"
#include "datastore.hpp"
#include "keys.hpp"
#include "mbed.h"
#include "smartlock.hpp"
#include "totp.hpp"
#include <chrono>

using namespace std::chrono_literals;

const char DEVICE_NAME[10] = "SmartLock";
static const uint16_t MAX_ADVERTISING_PAYLOAD_SIZE = 50;

/**
 * @brief Initialize the bluetooth server and input handler.
 *
 * @param event_queue The global event queue.
 * @return 0 upon success, -1 on error / failure.
 */
int init_bluetooth(events::EventQueue &event_queue, SmartLock *smart_lock);

#endif // BLE_SERVICE_H
