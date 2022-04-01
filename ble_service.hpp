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

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "mbed.h"
#include <chrono>

#include "smartlock.hpp"
#include "totp.hpp"

using namespace std::chrono_literals;

/**
 * @brief A handler for the BLE input event.
 */
class BLEInputHandler : private mbed::NonCopyable<BLEInputHandler>,
                        public ble::GattServer::EventHandler {
public:
  /**
   * @brief Construct a new BLE input handler object.
   *
   * @return Instance of BLEInputHandler.
   */
  BLEInputHandler(SmartLock *smart_lock);

  /**
   * @brief Called when the device starts advertising itself to others.
   *
   * @param ble The BLE singleton.
   * @param event_queue The global event queue.
   * @return Void.
   */
  void start(BLE &ble, events::EventQueue &event_queue);

private:
  /**
   * @brief The GATT Characteristic that communicates the input.
   */
  WriteOnlyArrayGattCharacteristic<uint8_t, 3> *_input_characteristic;
  SmartLock *_smart_lock;

  /**
   * This callback doesn't do anything right now except print whatever is
   * written.
   *
   * @param params Information about the characterisitc being updated.
   * @return Void.
   */
  void onDataWritten(const GattWriteCallbackParams &params) override;
};

/**
 * @brief Initialize the bluetooth server and input handler.
 *
 * @param event_queue The global event queue.
 * @return 0 upon sucess, -1 on error / failure.
 */
int init_bluetooth(events::EventQueue &event_queue, SmartLock *smart_lock);

#endif // BLE_SERVICE_H
