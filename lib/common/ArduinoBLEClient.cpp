/**
 * @file ArduinoBLEClient.cpp
 * @author your name (you@domain.com)
 * @brief This code is heavily inspired from https://randomnerdtutorials.com/esp32-ble-server-client/
 * @version 0.1
 * @date 2022-06-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "ArduinoBLEClient.h"
#include "Arduino.h"
using std::string;

bool doConnect;
bool connected;
bool searchFinished;
// Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

ArduinoBLEClient::ArduinoBLEClient() {}

ArduinoBLEClient::~ArduinoBLEClient() {}

BLEUUID serviceUUID;
BLEUUID triggerUUID;

char *temperatureChar;
bool newTemperature = false;

static BLERemoteCharacteristic *triggerCharacteristic;

const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

void ArduinoBLEClient::setup() {
    Serial.println("Setting up BLEClient");
    BLEDevice::setPower(ESP_PWR_LVL_P7);
    BLEDevice::init("");

    serviceUUID = BLEUUID::fromString(BLE_TRIGGER_SERVICE_UUID);
    triggerUUID = BLEUUID::fromString(BLE_TRIGGER_CHARACTERISTIC_UUID);

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 30 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(this);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30, false);
    searchFinished = true;
}

// When the BLE Server sends a new temperature reading with the notify property
static void triggerNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                                  uint8_t *pData, size_t length, bool isNotify) {
    // store temperature value
    temperatureChar = (char *)pData;
    newTemperature = true;
}

// Connect to the BLE Server that has the name, Service, and Characteristics
bool ArduinoBLEClient::connectToServer(BLEAddress pAddress) {
    // Block
    while (!searchFinished) {
        delay(100);
    }
    Serial.println("Creating client");
    BLEClient *pClient = BLEDevice::createClient();

    Serial.println("Created Client");
    delay(1000);

    // Connect to the remove BLE Server.
    pClient->connect(pAddress);
    Serial.println("Connected to server");
    delay(1000);

    std::map<std::string, BLERemoteService *> *services = pClient->getServices();
    Serial.println("Begin listing services");
    std::map<std::string, BLERemoteService *>::iterator it_c = services->begin();
    delay(100);

    Serial.println("[ArduinoBLEClient] List Characteristics");

    while (it_c != services->end()) {
        Serial.println(it_c->first.c_str());
        Serial.println(it_c->second->toString().c_str());
        it_c++;
    }

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    Serial.print("Our service UUID ");
    Serial.println(serviceUUID.toString().c_str());
    if (pRemoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        return (false);
    }
    Serial.println("Gotten service");
    delay(100);

    Serial.println("Begin listing characteristics");
    std::map<std::string, BLERemoteCharacteristic *> *characteristics = pRemoteService->getCharacteristics();
    std::map<std::string, BLERemoteCharacteristic *>::iterator it = characteristics->begin();

    Serial.println("[ArduinoBLEClient] List Characteristics");
    delay(100);

    while (it != characteristics->end()) {
        Serial.println(it->first.c_str());
        Serial.println(it->second->toString().c_str());
        it++;
    }

    triggerCharacteristic = pRemoteService->getCharacteristic(triggerUUID);
    Serial.print("Our characteristic UUID ");
    Serial.println(triggerUUID.toString().c_str());

    if (triggerCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID");
        return false;
    }
    Serial.println(" - Found our characteristics");

    // Assign callback functions for the Characteristics
    triggerCharacteristic->registerForNotify(triggerNotifyCallback);
    return true;
}

// function that prints the latest sensor readings in the OLED display
void ArduinoBLEClient::printReadings() {
    Serial.print("Temperature:");
    Serial.print(temperatureChar);
    Serial.println("%");
}

void ArduinoBLEClient::monitor() {
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true) {
        if (connectToServer(*pServerAddress)) {
            Serial.println("We are now connected to the BLE Server.");
            // Activate the Notify property of each Characteristic
            triggerCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t *)notificationOn, 2, true);
            connected = true;
        } else {
            Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
        }
        doConnect = false;
    }
    // if new temperature readings are available, print in the OLED
    if (newTemperature) {
        newTemperature = false;
        printReadings();
    }
}

void ArduinoBLEClient::onResult(BLEAdvertisedDevice advertisedDevice) {
    // Is this one of our devices?
    for (string supported_peripheral : SUPPORTED_PERIPHERALS)
        if (advertisedDevice.getName() == supported_peripheral) {           // Check if the name of the advertiser matches
            advertisedDevice.getScan()->stop();                             // TODO, probably remove to support multiple devices. Scan can be stopped, we found what we are looking for
            pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
            doConnect = true;                                               // Set indicator, stating that we are ready to connect
            Serial.println("Device found. Connecting!");
            Serial.println(advertisedDevice.getAddress().toString().c_str());
        }
}
