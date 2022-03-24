/**
 * @file smartlock.hpp
 * @author Kirill Tregubov (KirillTregubov), Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This header contains definitions of functions and classes for
 * operating Smart Lock.
 * @bug No known bugs.
 */
#ifndef SMART_LOCK
#define SMART_LOCK

#include "mbed.h"
// #include "ble/BLE.h"
// #include "ble/Gap.h"
#include "ntp-client/NTPClient.h"
#include <ctype.h>

/* BLE_Service */

/**
 * @brief A handler for the BLE input event.
 */
class BLEInputHandler : private mbed::NonCopyable<BLEInputHandler>, public ble::Gap::EventHandler
{
public:
	/**
     * @brief Construct a new BLE input handler object.
	 *
	 * @return Instance of BLEInputHandler.
     */
	BLEInputHandler();

    /**
     * @brief Called when the device starts advertising itself to others.
	 *
	 * @param ble The BLE singleton.
	 * @param event_queue The global event queue.
	 * @return Void.
     */
    void start(BLE &ble, events::EventQueue &event_queue);

	/**
	* This callback doesn't do anything right now except print whatever is
	* written.
	*
	* @param params Information about the characterisitc being updated.
	* @return Void.
	*/
	void onDataWritten(const ble::GattWriteCallbackParams &params) override;

private:
	/**
     * @brief The GATT Characteristic that communicates the input.
     */
	WriteOnlyGattCharacteristic<uint8_t> _input_characteristic;
};

/**
 * @brief Initialize the bluetooth server and input handler.
 *
 * @param event_queue The global event queue.
 * @return 0 upon sucess, -1 on error / failure.
 */
int init_bluetooth(events::EventQueue &event_queue);

/* WiFi_Service */

/**
 * @brief Connect to the WiFi network specified in 'mbed_app.json'.
 *
 * @param wifi A WiFiInterface instance.
 * @return 0 upon success, -1 on error / failure.
 */
int connect_to_wifi(WiFiInterface *wifi);

/* RTC_Service */

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

/* HELPERS */

/**
 * @brief Convert a string to uppercase.
 *
 * @param str The string that you want to convert to uppercase.
 * @return The address of the string.
 */
char *strupr(char *str);

#endif // SMART_LOCK
