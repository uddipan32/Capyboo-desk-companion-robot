#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE command length limits
// Default BLE MTU: 20 bytes per packet (BLE 4.0)
// With MTU negotiation (BLE 4.2+): up to 512 bytes
// ESP32 BLE library handles chunking automatically
// Practical limit: ~200-300 characters for most commands
#define BLE_MAX_COMMAND_LENGTH 256  // Maximum command length

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

// Buffer for accumulating multi-packet BLE data
String bleReceiveBuffer = "";
unsigned long bleLastReceiveTime = 0;
const unsigned long BLE_RECEIVE_TIMEOUT = 100; // 100ms timeout between packets

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
        unsigned long currentTime = millis();
        
        // Check if this is a continuation of previous data (within timeout)
        if (bleReceiveBuffer.length() > 0 && (currentTime - bleLastReceiveTime) < BLE_RECEIVE_TIMEOUT) {
          // Append to buffer (accumulating chunks)
          bleReceiveBuffer += rxValue;
          Serial.print("BLE chunk received (");
          Serial.print(rxValue.length());
          Serial.print(" bytes), buffer now: ");
          Serial.print(bleReceiveBuffer.length());
          Serial.println(" bytes");
        } else {
          // New command or timeout - start fresh buffer
          bleReceiveBuffer = rxValue;
          Serial.print("BLE new data received (");
          Serial.print(rxValue.length());
          Serial.println(" bytes)");
        }
        
        bleLastReceiveTime = currentTime;
        
        // Check if buffer exceeds limit
        if (bleReceiveBuffer.length() > BLE_MAX_COMMAND_LENGTH) {
          Serial.print("BLE command too long: ");
          Serial.print(bleReceiveBuffer.length());
          Serial.print(" bytes (max: ");
          Serial.print(BLE_MAX_COMMAND_LENGTH);
          Serial.println(")");
          bleReceiveBuffer = ""; // Clear buffer
          return;
        }
        
        // Check if command is complete (ends with newline or is a complete WiFi command)
        // For WiFi commands, check if we have both colons
        if (bleReceiveBuffer.indexOf('\n') >= 0 || 
            bleReceiveBuffer.indexOf('\r') >= 0 ||
            (bleReceiveBuffer.startsWith("wifi:") && bleReceiveBuffer.indexOf(':', 5) > 0)) {
          // Command is complete
          bleReceivedData = bleReceiveBuffer;
          bleReceivedData.trim();
          bleReceiveBuffer = ""; // Clear buffer
          
          Serial.print("BLE Complete command (");
          Serial.print(bleReceivedData.length());
          Serial.print(" bytes): ");
          Serial.println(bleReceivedData);
        }
        // Otherwise, wait for more chunks (will timeout in handleBLESerial)
      }
    }
};

// Initialize BLE Serial
void initBLESerial(const char* deviceName) {
  // Initialize BLE device
  BLEDevice::init(deviceName);
  
  // Set MTU size to allow larger packets (up to 512 bytes)
  // This enables BLE 4.2+ extended MTU feature
  BLEDevice::setMTU(512);
  
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
  // Set value size to allow longer commands
  pBLERxCharacteristic = pService->createCharacteristic(
                      BLE_CHAR_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  // Set the maximum value length (512 bytes)
  pBLERxCharacteristic->setValue((uint8_t*)NULL, 0);
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
  // Check for timeout on receive buffer (finalize command if no more data coming)
  if (bleReceiveBuffer.length() > 0) {
    unsigned long currentTime = millis();
    if ((currentTime - bleLastReceiveTime) >= BLE_RECEIVE_TIMEOUT) {
      // Timeout reached - finalize the command
      bleReceivedData = bleReceiveBuffer;
      bleReceivedData.trim();
      bleReceiveBuffer = ""; // Clear buffer
      
      Serial.print("BLE Command finalized after timeout (");
      Serial.print(bleReceivedData.length());
      Serial.print(" bytes): ");
      Serial.println(bleReceivedData);
    }
  }
  
  // Handle disconnection
  if (!bleDeviceConnected && bleOldDeviceConnected) {
    delay(500); // Give the bluetooth stack time to get ready
    pBLEServer->startAdvertising(); // Restart advertising
    Serial.println("BLE: Restarting advertising...");
    bleOldDeviceConnected = bleDeviceConnected;
    // Clear buffers on disconnect
    bleReceiveBuffer = "";
    bleReceivedData = "";
  }
  
  // Handle new connection
  if (bleDeviceConnected && !bleOldDeviceConnected) {
    Serial.println("BLE: Device connected!");
    bleOldDeviceConnected = bleDeviceConnected;
    
    // Clear buffers on new connection
    bleReceiveBuffer = "";
    bleReceivedData = "";
    
    // Send welcome message
    bleSerialPrintln("Robot connected. Commands: weather, animation, timer");
  }
}

#endif // BLUETOOTH_H

