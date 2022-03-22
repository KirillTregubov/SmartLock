#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "gatt_server_process.h"

class BLEPasswordListener : ble::GattServer::EventHandler {

    const static uint16_t EXAMPLE_SERVICE_UUID         = 0xA000;
    const static uint16_t WRITABLE_CHARACTERISTIC_UUID = 0xA001;

public:
    BLEPasswordListener()
    {
        const UUID uuid = WRITABLE_CHARACTERISTIC_UUID;
        _writable_characteristic = new ReadWriteGattCharacteristic<uint8_t> (uuid, &_characteristic_value);

        if (!_writable_characteristic) {
            printf("Allocation of ReadWriteGattCharacteristic failed\r\n");
        }
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = EXAMPLE_SERVICE_UUID;
        GattCharacteristic* charTable[] = { _writable_characteristic };
        GattService example_service(uuid, charTable, 1);

        ble.gattServer().addService(example_service);

        ble.gattServer().setEventHandler(this);

        printf("Service added with UUID 0xA000\r\n");
        printf("Connect and write to characteristic 0xA001\r\n");
    }

private:
    /**
     * This callback doesn't do anything right now except print whatever is written.
     *
     * @param[in] params Information about the characterisitc being updated.
     */
    virtual void onDataWritten(const GattWriteCallbackParams &params)
    {
        if (params.handle == _writable_characteristic->getValueHandle()) {
            printf("New characteristic value written: %x\r\n", *(params.data));
        }
    }

    ReadWriteGattCharacteristic<uint8_t> *_writable_characteristic = nullptr;
    uint8_t _characteristic_value = 0;
};

/**
     * Call this to start the bluetooth server and listener.
     *
     * @param[in] event_queue The global event queue.
*/
int init_bluetooth(EventQueue event_queue)
{
    BLE &ble = BLE::Instance();    

    printf("\r\nGATT server with one writable characteristic\r\n");

    BLEPasswordListener passwordListener;

    /* this process will handle basic setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    ble_process.on_init(callback(&passwordListener, &BLEPasswordListener::start));
    ble_process.start();

    return 0;
}