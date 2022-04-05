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
  uint8_t inputValue[6];
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

int digits_only(const char *s) {
  while (*s) {
    if (isdigit(*s++) == 0)
      return 0;
  }

  return 1;
}

void BLEInputHandler::onDataWritten(const GattWriteCallbackParams &params) {
  if (params.handle == _input_characteristic->getValueHandle()) {
    if (params.len != 3 and params.len != 6) {
      printf("> Received code has incorrect length\n");
      return;
    }

    char *code = (char *)malloc(7);
    if (params.len == 6) {
      int index = 0;
      for (int i = 0; i < params.len; i++) {
        index += sprintf(&code[index], "%c", params.data[i]);
      }
      code[6] = '\0';
    } else if (params.len == 3) {
      int index = 0;
      for (int i = 0; i < params.len; i++) {
        index += sprintf(&code[index], "%02x", params.data[i]);
      }
      code[6] = '\0';
    }

    printf("> Received code %s\n", code);

    if (_smart_lock->is_unlocked()) {
      printf("> SmartLock already unlocked\n");
      return;
    }

    char log_message[50];
    if (digits_only(code)) {
      char secret[PRIVATE_KEY_LENGTH + 1];
      get_private_key(secret);

      if (validate(secret, code)) {
        sprintf(log_message, "Received valid TOTP code: %s", code);
        write_log(log_message);
        _smart_lock->unlock();
      } else {
        printf("> Received code is incorrect\n");
        sprintf(log_message, "Received invalid TOTP code: %s", code);
        write_log(log_message);
      }
    } else {
      char secret[RECOVERY_KEY_LENGTH + 1];
      get_recovery_keys(secret);

      char recovery_key[7];
      for (int key = 0; key < RECOVERY_KEY_LENGTH; key += 6) {
        strncpy(recovery_key, secret + key, 6);
        recovery_key[6] = '\0';

        if (strcmp(recovery_key, code) == 0) {
          printf("> Validated successfully!\n");
          sprintf(log_message, "Received valid recovery code: %s", code);
          write_log(log_message);
          _smart_lock->unlock();

          strncpy(secret + key, "000000", 6);
          set_recovery_keys(secret);
          sprintf(log_message, "Removed recovery code: %s", code);
          return;
        }
      }

      printf("> Received incorrect code\n");
      sprintf(log_message, "Received invalid recovery code: %s", code);
      write_log(log_message);
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
