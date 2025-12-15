#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE Serial Port Profile UUIDs
#define BLE_SERVICE_UUID        "0000fff0-0000-1000-8000-00805f9b34fb"  // Serial Port Service
#define BLE_CHAR_UUID_TX        "0000fff1-0000-1000-8000-00805f9b34fb"  // TX Characteristic
#define BLE_CHAR_UUID_RX        "0000fff2-0000-1000-8000-00805f9b34fb"  // RX Characteristic

// BLE objects
BLEServer* pBLEServer = NULL;
BLECharacteristic* pBLETxCharacteristic = NULL;
BLECharacteristic* pBLERxCharacteristic = NULL;
bool bleDeviceConnected = false;
bool bleOldDeviceConnected = false;
String bleReceivedData = "";

// BLE Server Callbacks
class MyBLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleDeviceConnected = true;
      Serial.println("BLE Device connected");
    }

    void onDisconnect(BLEServer* pServer) {
      bleDeviceConnected = false;
      Serial.println("BLE Device disconnected");
    }
};

// BLE Characteristic Callbacks for RX (receiving data)
class BLERxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        bleReceivedData = rxValue;
        bleReceivedData.trim();
        bleReceivedData.toLowerCase();
        Serial.print("BLE Received: ");
        Serial.println(bleReceivedData);
      }
    }
};

// Initialize BLE Serial
void initBLESerial(const char* deviceName) {
  // Initialize BLE device
  BLEDevice::init(deviceName);
  
  // Create BLE server
  pBLEServer = BLEDevice::createServer();
  pBLEServer->setCallbacks(new MyBLEServerCallbacks());

  // Create BLE Serial Port Service
  BLEService *pService = pBLEServer->createService(BLE_SERVICE_UUID);

  // Create TX Characteristic (for sending data to phone)
  pBLETxCharacteristic = pService->createCharacteristic(
                      BLE_CHAR_UUID_TX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pBLETxCharacteristic->addDescriptor(new BLE2902());

  // Create RX Characteristic (for receiving data from phone)
  pBLERxCharacteristic = pService->createCharacteristic(
                      BLE_CHAR_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  pBLERxCharacteristic->setCallbacks(new BLERxCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // Helps with iPhone connections
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.print("BLE Serial started! Device name: ");
  Serial.println(deviceName);
  Serial.println("Waiting for connection...");
}

// Check if data is available
bool bleSerialAvailable() {
  return (bleReceivedData.length() > 0);
}

// Read received data
String bleSerialRead() {
  String data = bleReceivedData;
  bleReceivedData = ""; // Clear after reading
  return data;
}

// Send data via BLE
void bleSerialPrint(String data) {
  if (bleDeviceConnected && pBLETxCharacteristic != NULL) {
    pBLETxCharacteristic->setValue(data.c_str());
    pBLETxCharacteristic->notify();
    delay(10); // Small delay to ensure notification is sent
  }
}

void bleSerialPrintln(String data) {
  bleSerialPrint(data + "\n");
}

// Handle BLE connection/disconnection (call this in loop)
void handleBLESerial() {
  // Handle disconnection
  if (!bleDeviceConnected && bleOldDeviceConnected) {
    delay(500); // Give the bluetooth stack time to get ready
    pBLEServer->startAdvertising(); // Restart advertising
    Serial.println("BLE: Restarting advertising...");
    bleOldDeviceConnected = bleDeviceConnected;
  }
  
  // Handle new connection
  if (bleDeviceConnected && !bleOldDeviceConnected) {
    Serial.println("BLE: Device connected!");
    bleOldDeviceConnected = bleDeviceConnected;
    
    // Send welcome message
    bleSerialPrintln("Robot connected. Commands: weather, animation, timer");
  }
}

#endif // BLUETOOTH_H

