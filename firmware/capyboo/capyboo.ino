#include "bluetooth.h"
#include "display.h"
#include "wifi.h"
#include "mqtt.h"
#include <Wire.h>

#include "face_animation.h"
#include "secrets.h"  // WiFi and MQTT credentials

// Touch sensor pin (from definitions.h)
const int TOUCH_SENSOR_PIN = 4;


void setup() {
    Serial.begin(115200);
    
    // Initialize BLE Serial
    initBLESerial("Capyboo");

    // Initialize touch sensor
    pinMode(TOUCH_SENSOR_PIN, INPUT);

    // Initialize display
    Wire.begin();
    if (!display.begin(0x3C, true)) {
        Serial.println(F("SH1106 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(3);
    
    // Center the text
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds("Capyboo", 0, 0, &x1, &y1, &w, &h);
    int16_t x = (128 - w) / 2;
    int16_t y = (64 - h) / 2;
    display.setCursor(x, y);
    display.println("Capyboo");
    display.display();
    delay(2000);

     // Connect to WiFi
    if (!connectWifi(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("Failed to connect to WiFi");
        display_text("Failed to connect to WiFi");
        delay(2000);
    } else {
        Serial.println("Connected to WiFi");
        String wifiMsg = "Connected to WiFi: " + String(WIFI_SSID);
        display_text(wifiMsg.c_str());
        delay(2000);
        
    }

    // Initialize MQTT
    if (initMQTT()) {
        if (connectMQTT()) {
            Serial.println("MQTT connected successfully");
            display_text("MQTT connected");
            delay(2000);
            publishStatus("Capyboo started and connected");
            
        } else {
            Serial.println("Failed to connect to MQTT broker");
            display_text("MQTT failed");
            delay(2000);
        }
    }
    
    // Play wakeup animation once at startup
    playWakeupAnimation();
}

// Animation sequence state
int animationIndex = 0;
unsigned long lastAnimationTime = 0;
const unsigned long ANIMATION_DELAY = 2000; // Delay between animations in ms

void loop() {
    // Handle BLE connection/disconnection (required for BLE communication)
    handleBLESerial();
    
    // Handle MQTT connection and messages (must be called regularly)
    handleMQTT();
    
    // Check for MQTT messages first (high priority)
    if (mqttMessageAvailable()) {
        String command = getMQTTMessage();
        command.toLowerCase();
        command.trim();
        
        Serial.print("Received MQTT command: ");
        Serial.println(command);

        // Display the received command on screen immediately
        display_text(command.c_str());
        delay(2000);
        
        // Handle commands
        if (command == "wakeup" || command == "start") {
            playWakeupAnimation();
            publishStatus("Wakeup animation played");
        } else if (command == "lookright") {
            playLookRightFromMiddleAnimation();
            publishStatus("Looking right");
        } else if (command == "lookleft") {
            playLookLeftFromMiddleAnimation();
            publishStatus("Looking left");
        } else if (command == "tongue") {
            playTongueOutAnimation();
            publishStatus("Tongue out");
        } else if (command == "funnyeyes") {
            playNormalToFunnyEyesAnimation();
            publishStatus("Funny eyes activated");
        } else {
            String errorMsg = "Unknown: " + command;
            display_text(errorMsg.c_str());
            delay(2000);
            publishStatus(errorMsg.c_str());
        }
        // Reset animation timing after handling MQTT message
        lastAnimationTime = millis();
    }
    
    // Play animation sequence continuously, checking MQTT between animations
    unsigned long currentTime = millis();
    if (currentTime - lastAnimationTime >= ANIMATION_DELAY) {
        // Check MQTT again before playing animation
        handleMQTT();
        
        // Play next animation in sequence
        switch (animationIndex) {
            case 0:
                playLookRightFromMiddleAnimation();
                break;
            case 1:
                playLookMiddleFromRightAnimation();
                break;
            case 2:
                playLookLeftFromMiddleAnimation();
                break;
            case 3:
                playLookMiddleFromLeftAnimation();
                break;
            case 4:
                playNormalToFunnyEyesAnimation();
                break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
                playTongueOutAnimation();
                break;
            case 18:
                playFunnyEyesToNormalAnimation();
                break;
            default:
                animationIndex = -1; // Reset to start
                break;
        }
        
        animationIndex++;
        if (animationIndex > 18) {
            animationIndex = 0; // Loop back to start
        }
        
        lastAnimationTime = currentTime;
    }
}

