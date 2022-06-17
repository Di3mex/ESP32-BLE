#pragma once

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <string.h>

class ArduinoBLEClient : public BLEAdvertisedDeviceCallbacks {
  private:
    // Currently up to 4 devices are supported
    BLEDevice devices[4];
    // const std::string BLE_SERVER_NAME = "BLE_ESP32_MCU";

    bool connectToServer(BLEAddress pAddress);

  public:
    const std::string SUPPORTED_PERIPHERALS[3] = {"FSR-PERIPHERAL", "BTN-PERIPHERAL", "EMG-PERIPHERAL"};

    const std::string BLE_TRIGGER_SERVICE_UUID = "000000ff-0000-1000-8000-00805f9b34fb";
    const std::string BLE_TRIGGER_CHARACTERISTIC_UUID = "0000ff01-0000-1000-8000-00805f9b34fb";

    ArduinoBLEClient();
    ~ArduinoBLEClient();

    /**
     * @brief Called once at the beginning to setup the BLE controller
     */
    void setup();

    /**
     * @brief Called from a loop/task to check up on the BLE devices
     *
     */
    void monitor();

    void printReadings();

    /**
     * @brief Callback from the Arduino BLE libray. Called when a new device is discovered
     */
    void onResult(BLEAdvertisedDevice advertisedDevice);
};
