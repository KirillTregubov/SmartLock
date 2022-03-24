#include "ble/BLE.h"
// #include "gatt_server_process.h"
#include "ble_process.h"
#include <events/mbed_events.h>

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

class BLEInputHandler : ble::GattServer::EventHandler {

  const uint16_t INPUT_SERVICE_UUID = 0xA000;
  const uint16_t INPUT_CHARACTERISTIC_UUID = 0xA001;

public:
  BLEInputHandler() {
    static uint8_t inputValue[10] = {0};
    _input_characteristic = new WriteOnlyGattCharacteristic<uint8_t>(INPUT_CHARACTERISTIC_UUID, inputValue);

    if (!_input_characteristic) {
      printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
    }
  }

  void start(BLE &ble, events::EventQueue &event_queue) {
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
    GattService input_service(INPUT_SERVICE_UUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

    ble.gattServer().addService(input_service);

    ble.gattServer().setEventHandler(this);

    printf("Service added with UUID 0xA000\r\n");
    printf("Connect and write to characteristic 0xA001\r\n");
  }

private:
  /**
   * This callback doesn't do anything right now except print whatever is
   * written.
   *
   * @param[in] params Information about the characterisitc being updated.
   */
  void onDataWritten(const GattWriteCallbackParams &params) {
    if (params.handle == _input_characteristic->getValueHandle()) {
      printf("New characteristic value written: %x\r\n", *(params.data));
    }
  }

  WriteOnlyGattCharacteristic<uint8_t> *_input_characteristic = nullptr;
};

/**
 * Call this to start the bluetooth server and listener.
 *
 * @param[in] event_queue The global event queue.
 */
int init_bluetooth(events::EventQueue &event_queue) {
  BLE &ble = BLE::Instance();

  printf("\r\nGATT server with one writable characteristic\r\n");

  BLEInputHandler inputHandler;

  SmartLockBLEProcess ble_process(event_queue, ble);

  ble_process.on_init(callback(&inputHandler, &BLEInputHandler::start));

  ble_process.start();

  return 0;
}
