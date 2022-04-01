/**
 * @file ble_service.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functionality for initializing and using BLE.
 * @bug No known bugs.
 */
#include "ble_service.hpp"
#include "ble_process.h"

/**
 * Simple GattServer wrapper. It will advertise and allow a connection.
 */
class SmartLockBLEProcess : public BLEProcess {
public:
  SmartLockBLEProcess(events::EventQueue &event_queue, BLE &ble_interface)
      : BLEProcess(event_queue, ble_interface) {}

  const char *get_device_name() override {
    static const char name[] = "SmartLock";
    return name;
  }
};

BLEInputHandler::BLEInputHandler(SmartLock *smart_lock) {
  uint8_t inputValue[3];
  _input_characteristic =
      new WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(inputValue)>(
          0xA000, inputValue);
  _smart_lock = smart_lock;

  if (!_input_characteristic) {
    printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
  }
}

void BLEInputHandler::start(BLE &ble, events::EventQueue &event_queue) {
  // Setup the default phy used in connection to 2M for faster transfer
  auto &gap = ble.gap();
  if (gap.isFeatureSupported(ble::controller_supported_features_t::LE_2M_PHY)) {
    ble::phy_set_t phys(false, true, false);

    ble_error_t error = gap.setPreferredPhys(&phys, &phys);
    if (error) {
      print_error(error, "GAP::setPreferedPhys failed");
    }
  }

  GattCharacteristic *characteristics[] = {_input_characteristic};
  GattService input_service(0xA001, characteristics, 1);

  ble.gattServer().addService(input_service);
  ble.gattServer().setEventHandler(this);
}

void BLEInputHandler::onDataWritten(const GattWriteCallbackParams &params) {
  if (params.handle == _input_characteristic->getValueHandle()) {
    if (params.len != 3) {
      printf("> Received code has incorrect length\n");
      return;
    }
    char code[6];
    int index = 0;
    for (int i = 0; i < params.len; i++) {
      index += sprintf(&code[index], "%02x", params.data[i]);
    }
    printf("> Received code %s\n\r", code);

    if (validate("569861750830A66BEBFF", code)) {
      printf(">Validated successfully!\n");
      _smart_lock->unlock();
    } else {
      printf("> Code is incorrect\n");
    }
  }
}

int init_bluetooth(events::EventQueue &event_queue, SmartLock *smart_lock) {

  BLE &ble = BLE::Instance();

  printf("\r\nGATT server with one writable characteristic\r\n");

  BLEInputHandler inputHandler(smart_lock);

  SmartLockBLEProcess ble_process(event_queue, ble);

  ble_process.on_init(callback(&inputHandler, &BLEInputHandler::start));

  ble_process.start();

  return 0;
}
