# Installing Required Libraries for ESP32

## PubSubClient Library Installation

### Method 1: Arduino IDE Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Tools → Manage Libraries...** (or press `Ctrl+Shift+I`)
3. In the search box, type: **PubSubClient**
4. Find **"PubSubClient" by Nick O'Leary**
5. Click **Install**
6. Wait for installation to complete

### Method 2: Manual Installation

If Library Manager doesn't work, you can install manually:

1. Download the library from: https://github.com/knolleary/pubsubclient/releases
2. Extract the ZIP file
3. In Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library...**
4. Select the extracted folder
5. Restart Arduino IDE

## Required Libraries Summary

- **PubSubClient** by Nick O'Leary (for MQTT)
- **WiFi** (built-in with ESP32)
- **WiFiClientSecure** (built-in with ESP32)
- **Adafruit GFX Library** (for display)
- **Adafruit SH110X** (for SH1106 display)
- **BLE libraries** (built-in with ESP32)

## ESP32 Board Support

Make sure you have ESP32 board support installed:

1. Go to **File → Preferences**
2. In "Additional Boards Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for "ESP32" and install "esp32 by Espressif Systems"
