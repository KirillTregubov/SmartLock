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

/**
 * @brief Print BLE error.
 *
 * From the BLE Utils library
 * https://github.com/ARMmbed/mbed-os-ble-utils
 *
 * @author ARMmbed
 * @copyright Copyright (c) ARM Limited. (Apache-2.0 License)
 */
inline void print_ble_error(ble_error_t error, const char *msg) {
  printf("%s: ", msg);
  switch (error) {
  case BLE_ERROR_NONE:
    printf("BLE_ERROR_NONE: No error");
    break;
  case BLE_ERROR_BUFFER_OVERFLOW:
    printf("BLE_ERROR_BUFFER_OVERFLOW: The requested action would cause a "
           "buffer overflow and has been aborted");
    break;
  case BLE_ERROR_NOT_IMPLEMENTED:
    printf("BLE_ERROR_NOT_IMPLEMENTED: Requested a feature that isn't yet "
           "implement or isn't supported by the target HW");
    break;
  case BLE_ERROR_PARAM_OUT_OF_RANGE:
    printf("BLE_ERROR_PARAM_OUT_OF_RANGE: One of the supplied parameters is "
           "outside the valid range");
    break;
  case BLE_ERROR_INVALID_PARAM:
    printf(
        "BLE_ERROR_INVALID_PARAM: One of the supplied parameters is invalid");
    break;
  case BLE_STACK_BUSY:
    printf("BLE_STACK_BUSY: The stack is busy");
    break;
  case BLE_ERROR_INVALID_STATE:
    printf("BLE_ERROR_INVALID_STATE: Invalid state");
    break;
  case BLE_ERROR_NO_MEM:
    printf("BLE_ERROR_NO_MEM: Out of Memory");
    break;
  case BLE_ERROR_OPERATION_NOT_PERMITTED:
    printf("BLE_ERROR_OPERATION_NOT_PERMITTED");
    break;
  case BLE_ERROR_INITIALIZATION_INCOMPLETE:
    printf("BLE_ERROR_INITIALIZATION_INCOMPLETE");
    break;
  case BLE_ERROR_ALREADY_INITIALIZED:
    printf("BLE_ERROR_ALREADY_INITIALIZED");
    break;
  case BLE_ERROR_UNSPECIFIED:
    printf("BLE_ERROR_UNSPECIFIED: Unknown error");
    break;
  case BLE_ERROR_INTERNAL_STACK_FAILURE:
    printf("BLE_ERROR_INTERNAL_STACK_FAILURE: internal stack failure");
    break;
  case BLE_ERROR_NOT_FOUND:
    printf("BLE_ERROR_NOT_FOUND");
    break;
  default:
    printf("Unknown error");
  }
  printf("\n");
}

/**
 * @brief Handle initialization and shutdown of the BLE Instance.
 *
 * Call start() to initiate processing.
 *
 * Based on the BLE Utils library
 * https://github.com/ARMmbed/mbed-os-ble-utils
 *
 * @author ARMmbed
 * @copyright Copyright (c) ARM Limited. (Apache-2.0 License)
 */
class SmartLockBLEProcess : private mbed::NonCopyable<SmartLockBLEProcess>,
                            public ble::Gap::EventHandler {
public:
  /**
   * @brief Construct a BLEProcess from an event queue and a ble interface.
   */
  SmartLockBLEProcess(events::EventQueue &event_queue, BLE &ble_interface)
      : _event_queue(event_queue), _ble(ble_interface),
        _gap(ble_interface.gap()), _adv_data_builder(_adv_buffer) {}

  ~SmartLockBLEProcess() { stop(); }

  /**
   * @brief Initialize the ble interface, configure it and start advertising.
   */
  void start() {
    if (_ble.hasInitialized()) {
      printf("Error: the ble instance has already been initialized.\n");
      return;
    }

    _gap.setEventHandler(this);
    _ble.onEventsToProcess(
        makeFunctionPointer(this, &SmartLockBLEProcess::schedule_ble_events));

    ble_error_t error = _ble.init(this, &SmartLockBLEProcess::on_init_complete);
    if (error) {
      print_ble_error(error, "Error returned by BLE::init.\n");
      return;
    }

    _event_queue.dispatch_forever();
    return;
  }

  /**
   * @brief Close existing connections and stop the process.
   */
  void stop() {
    if (_ble.hasInitialized()) {
      _ble.shutdown();
      printf("> BLE stopped");
    }
  }

  /**
   * @brief Subscription to the ble interface initialization event.
   *
   * @param[in] cb The callback object that will be called when the ble
   * interface is initialized.
   */
  void on_init(mbed::Callback<void(BLE &, events::EventQueue &)> cb) {
    _post_init_cb = cb;
  }

protected:
  /**
   * @brief Sets up adverting payload and start advertising.
   * This function is invoked when the ble interface is initialized.
   */
  void on_init_complete(BLE::InitializationCompleteCallbackContext *event) {
    if (event->error) {
      print_ble_error(event->error, "Error during the initialisation\n");
      return;
    }

    // Setup the default phy used in connection to 2M for faster transfer
    BLE &ble = BLE::Instance();
    auto &gap = ble.gap();
    if (gap.isFeatureSupported(
            ble::controller_supported_features_t::LE_2M_PHY)) {
      ble::phy_set_t phys(false, true, false);

      ble_error_t error = gap.setPreferredPhys(&phys, &phys);
      if (error) {
        print_ble_error(error, "GAP::setPreferedPhys failed\n");
      }
    }

    start_activity();

    if (_post_init_cb) {
      _post_init_cb(_ble, _event_queue);
    }
  }

  /**
   * @brief Start the gatt client process when a connection event is received.
   * This is called by Gap to notify the application we connected
   */
  void
  onConnectionComplete(const ble::ConnectionCompleteEvent &event) override {
    if (event.getStatus() == BLE_ERROR_NONE) {
      ble::address_t addr = event.getPeerAddress();
      printf("> Connected to %02x:%02x:%02x:%02x:%02x:%02x using BLE\n",
             addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

      char log_message[50];
      sprintf(log_message,
              "Device %02x:%02x:%02x:%02x:%02x:%02x connected using BLE",
              addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
      write_log(log_message);
    } else {
      printf("Failed to connect\r\n");
      start_activity();
    }
  }

  /**
   * @brief Stop the gatt client process when the device is disconnected then
   * restart advertising. This is called by Gap to notify the application we
   * disconnected
   */
  void onDisconnectionComplete(
      const ble::DisconnectionCompleteEvent &event) override {
    printf("> Device disconnected using BLE\n");
    write_log("Device disconnected using BLE");
    start_activity();
  }

  /**
   * @brief Restarts main activity
   */
  void onAdvertisingEnd(const ble::AdvertisingEndEvent &event) override {
    start_activity();
  }

  /**
   * @brief Start advertising or scanning. Triggered by init or disconnection.
   */
  virtual void start_activity() {
    _event_queue.call([this]() { start_advertising(); });
  }

  /**
   * @brief Start the advertising process; it ends when a device connects.
   */
  void start_advertising() {
    ble_error_t error;

    if (_gap.isAdvertisingActive(_adv_handle)) {
      return;
    }

    ble::AdvertisingParameters adv_params(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(40)));

    error = _gap.setAdvertisingParameters(_adv_handle, adv_params);
    if (error) {
      printf("_ble.gap().setAdvertisingParameters() failed\r\n");
      return;
    }

    _adv_data_builder.clear();
    _adv_data_builder.setFlags();
    _adv_data_builder.setName(DEVICE_NAME);

    error = _gap.setAdvertisingPayload(_adv_handle,
                                       _adv_data_builder.getAdvertisingData());
    if (error) {
      print_ble_error(error, "Gap::setAdvertisingPayload() failed\r\n");
      return;
    }

    error = _gap.startAdvertising(
        _adv_handle, ble::adv_duration_t(ble::millisecond_t(10000)));
    if (error) {
      print_ble_error(error, "Gap::startAdvertising() failed\r\n");
      return;
    }

    printf("> Advertising as \"%s\"\r\n", DEVICE_NAME);
  }

  /**
   * @brief Schedule processing of events from the BLE middleware in the event
   * queue.
   */
  void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event) {
    _event_queue.call(mbed::callback(&event->ble, &BLE::processEvents));
  }

protected:
  events::EventQueue &_event_queue;
  BLE &_ble;
  ble::Gap &_gap;

  uint8_t _adv_buffer[MAX_ADVERTISING_PAYLOAD_SIZE];
  ble::AdvertisingDataBuilder _adv_data_builder;

  ble::advertising_handle_t _adv_handle = ble::LEGACY_ADVERTISING_HANDLE;

  mbed::Callback<void(BLE &, events::EventQueue &)> _post_init_cb;
};

/**
 * @brief Checks if string contains only digits.
 *
 * @return 1 if string contains only digits, 0 otherwise
 */
int digits_only(const char *s) {
  while (*s) {
    if (isdigit(*s++) == 0)
      return 0;
  }

  return 1;
}

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
  BLEInputHandler(SmartLock *smart_lock) {
    uint8_t inputValue[6];
    _input_characteristic =
        new WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(inputValue)>(
            0xA000, inputValue);
    _smart_lock = smart_lock;

    if (!_input_characteristic) {
      printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
    }
  }

  /**
   * @brief Called when the device starts advertising itself to others.
   *
   * @param ble The BLE singleton.
   * @param event_queue The global event queue.
   * @return Void.
   */
  void start(BLE &ble, events::EventQueue &event_queue) {
    GattCharacteristic *characteristics[] = {_input_characteristic};
    GattService input_service(0xA001, characteristics, 1);

    ble.gattServer().addService(input_service);
    ble.gattServer().setEventHandler(this);
  }

private:
  /**
   * @brief The GATT Characteristic that communicates the input.
   */
  WriteOnlyArrayGattCharacteristic<uint8_t, 6> *_input_characteristic;
  SmartLock *_smart_lock;

  /**
   * This callback doesn't do anything right now except print whatever is
   * written.
   *
   * @param params Information about the characterisitc being updated.
   * @return Void.
   */
  void onDataWritten(const GattWriteCallbackParams &params) override {
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
};

int init_bluetooth(events::EventQueue &event_queue, SmartLock *smart_lock) {
  BLE &ble = BLE::Instance();
  BLEInputHandler inputHandler(smart_lock);
  SmartLockBLEProcess ble_process(event_queue, ble);

  ble_process.on_init(callback(&inputHandler, &BLEInputHandler::start));
  ble_process.start();
  return 0;
}
