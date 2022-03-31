/**
 * @file ble_service.cpp
 * @author Kirill Tregubov (KirillTregubov), Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functions for using BLE.
 * @bug No known bugs.
 */
#include "smartlock.hpp"
// #include "gatt_server_process.h"
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

BLEInputHandler::BLEInputHandler() {
  static uint8_t inputValue[10];
  _input_characteristic =
      new WriteOnlyGattCharacteristic<uint8_t>(0xA000, inputValue);

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

//   ble.gattClient().onDataWritten(&BLEInputHandler::onDataWritten);

  printf("Service added with UUID 0xA000\r\n");
  printf("Connect and write to characteristic 0xA001\r\n");
}

void BLEInputHandler::onDataWritten(const GattWriteCallbackParams &params) {
    printf("DATA WRITTEN\n");
  if (params.handle == _input_characteristic->getValueHandle()) {
    printf("Data received: length = %d, data = 0x", params.len);
    for (int x = 0; x < params.len; x++) {
      printf("%x", params.data[x]);
    }
    printf("\n\r");
  }
}

int init_bluetooth(events::EventQueue &event_queue) {
  BLE &ble = BLE::Instance();

  printf("\r\nGATT server with one writable characteristic\r\n");

  BLEInputHandler inputHandler;

  SmartLockBLEProcess ble_process(event_queue, ble);

  ble_process.on_init(callback(&inputHandler, &BLEInputHandler::start));

  ble_process.start();

  return 0;
}
