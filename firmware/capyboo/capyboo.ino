#include "bluetooth.h"
#include "display.h"
#include "wifi.h"
#include "wifi_storage.h"
#include "mqtt.h"
#include <Wire.h>

#include "face_animation.h"
#include "weather.h"
#include "dino_game.h"
#include "secrets.h"  // MQTT credentials (WiFi now stored in EEPROM)

// Touch sensor pin (from definitions.h)
const int TOUCH_SENSOR_PIN = 4;

// Mode definitions
enum Mode {
    MODE_ANIMATION,
    MODE_WEATHER,
    MODE_GAME
};

Mode currentMode = MODE_ANIMATION;  // Default mode


void setup() {
    Serial.begin(115200);
    
    // Initialize WiFi storage
    initWiFiStorage();
    
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

    // Try to connect to WiFi using stored credentials
    char storedSSID[64] = "";
    char storedPassword[64] = "";
    bool wifiConnected = false;
    
    if (loadWiFiCredentials(storedSSID, storedPassword, 64)) {
        Serial.println("Found stored WiFi credentials");
        display_text("Connecting...");
        wifiConnected = connectWifi(storedSSID, storedPassword);
    } else {
        // Try using secrets.h as fallback (for first-time setup)
        // Serial.println("No stored WiFi credentials, trying secrets.h");
        // if (strlen(WIFI_SSID) > 0) {
        //     display_text("Connecting...");
        //     wifiConnected = connectWifi(WIFI_SSID, WIFI_PASSWORD);
        //     // Save to storage if connection successful
        //     if (wifiConnected) {
        //         saveWiFiCredentials(WIFI_SSID, WIFI_PASSWORD);
        //     }
        // }
    }
    
    if (wifiConnected) {
        Serial.println("Connected to WiFi");
        String wifiMsg = "WiFi: " + String(storedSSID[0] != '\0' ? storedSSID : WIFI_SSID);
        display_text(wifiMsg.c_str());
        delay(2000);
    } else {
        Serial.println("WiFi not connected");
        display_text("WiFi: Not connected\nUse BLE to setup");
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
    
    // Display current mode
    displayCurrentMode();
}

// Function to display current mode
void displayCurrentMode() {
    String modeText = "Mode: ";
    switch (currentMode) {
        case MODE_ANIMATION:
            modeText += "Animation";
            break;
        case MODE_WEATHER:
            modeText += "Weather";
            break;
        case MODE_GAME:
            modeText += "Game";
            break;
    }
    display_text(modeText.c_str());
    delay(2000);
}

// Animation sequence state
int animationIndex = 0;
unsigned long lastAnimationTime = 0;
const unsigned long ANIMATION_DELAY = 0; // Delay between animations in ms

void loop() {
    // Handle BLE connection/disconnection (required for BLE communication)
    handleBLESerial();

    if (bleSerialAvailable()) {
        String command = bleSerialRead();
        command.trim();
        String lowerCommand = command;
        lowerCommand.toLowerCase();
        
        Serial.print("Received BLE command: ");
        Serial.println(command);
        display_text(command.c_str());
        delay(2000);
        
        // Handle WiFi setup command: "wifi:SSID:password"
        if (lowerCommand.startsWith("wifi:")) {
            int firstColon = command.indexOf(':');
            int secondColon = command.indexOf(':', firstColon + 1);
            
            if (secondColon > 0) {
                String ssid = command.substring(firstColon + 1, secondColon);
                String password = command.substring(secondColon + 1);
                
                Serial.print("Setting WiFi: SSID=");
                Serial.print(ssid);
                Serial.println(" (password hidden)");
                
                display_text("Setting WiFi...");
                delay(1000);
                
                // Save credentials
                if (saveWiFiCredentials(ssid.c_str(), password.c_str())) {
                    // Try to connect
                    if (connectWifi(ssid.c_str(), password.c_str())) {
                        display_text("WiFi connected!");
                        bleSerialPrintln("WiFi connected successfully!");
                        delay(2000);
                        
                        // Reconnect MQTT if WiFi is now available
                        if (initMQTT()) {
                            connectMQTT();
                        }
                    } else {
                        display_text("WiFi failed!");
                        bleSerialPrintln("WiFi connection failed. Credentials saved.");
                        delay(2000);
                    }
                } else {
                    display_text("Save failed!");
                    bleSerialPrintln("Failed to save WiFi credentials");
                    delay(2000);
                }
            } else {
                bleSerialPrintln("Invalid format. Use: wifi:SSID:password");
            }
        }
        // Handle clear WiFi command
        else if (lowerCommand == "clearwifi" || lowerCommand == "wificlear") {
            clearWiFiCredentials();
            disconnectWifi();
            display_text("WiFi cleared");
            bleSerialPrintln("WiFi credentials cleared");
            delay(2000);
        }
        // Handle mode switching commands
        else if (lowerCommand.startsWith("mode:")) {
            String modeStr = lowerCommand.substring(5); // Get text after "mode:"
            modeStr.trim();
            
            if (modeStr == "weather") {
                currentMode = MODE_WEATHER;
                bleSerialPrintln("Switched to Weather mode");
                displayCurrentMode();
            } else if (modeStr == "game") {
                currentMode = MODE_GAME;
                bleSerialPrintln("Switched to Game mode");
                displayCurrentMode();
            } else if (modeStr == "animation") {
                currentMode = MODE_ANIMATION;
                bleSerialPrintln("Switched to Animation mode");
                displayCurrentMode();
            } else {
                String errorMsg = "Unknown mode: " + modeStr;
                display_text(errorMsg.c_str());
                bleSerialPrintln("Unknown mode. Use: mode:animation, mode:weather, or mode:game");
                delay(2000);
            }
        }
        // Handle other commands
        else {
            display_text(command.c_str());
            delay(2000);
        }
    }

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
        } else if (command.startsWith("mode:")) {
            // Handle mode switching via MQTT
            String modeStr = command.substring(5);
            modeStr.trim();
            if (modeStr == "weather") {
                currentMode = MODE_WEATHER;
                displayCurrentMode();
                publishStatus("Switched to Weather mode");
            } else if (modeStr == "game") {
                currentMode = MODE_GAME;
                displayCurrentMode();
                publishStatus("Switched to Game mode");
            } else if (modeStr == "animation") {
                currentMode = MODE_ANIMATION;
                displayCurrentMode();
                publishStatus("Switched to Animation mode");
            } else {
                String errorMsg = "Unknown mode: " + modeStr;
                display_text(errorMsg.c_str());
                delay(2000);
                publishStatus(errorMsg.c_str());
            }
        } else {
            String errorMsg = "Unknown: " + command;
            display_text(errorMsg.c_str());
            delay(2000);
            publishStatus(errorMsg.c_str());
        }
        // Reset animation timing after handling MQTT message
        lastAnimationTime = millis();
    }

    // Handle different modes
    switch (currentMode) {
        case MODE_ANIMATION: {
            playTickleStartAnimation();
            playTickleAnimation();
            playTickleAnimation();
            playTickleAnimation();
            playTickleAnimation();
            playTickleAnimation();
            playTickleAnimation();
            playTickleEndAnimation();
            // Animation mode - play animation sequence
            // playIdleToSadAnimation();
            // playSadToCryAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryingAnimation();
            // playCryToSadAnimation();
            // playSadToIdleAnimation();
            // playEnjoyStartAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyingAnimation();
            // playEnjoyEndAnimation();
            // playIdleToHappyAnimation();
            // delay(2000);
            // playHappyToIdleAnimation();
            // delay(2000);
            // playIdleToAngryAnimation();
            // delay(2000);
            // playAngryToIdleAnimation();
            unsigned long currentTime = millis();

            if (currentTime - lastAnimationTime >= ANIMATION_DELAY) {
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
                    playTongueOutAnimation();
                        break;
                    case 6:
                    playTongueOutAnimation();
                        break;
                    case 7:
                    playTongueOutAnimation();
                        break;
                    case 8:
                    playTongueOutAnimation();
                        break;
                    case 9:
                    playTongueOutAnimation();
                        break;
                    case 10:
                    playTongueOutAnimation();
                        break;
                    case 11:
                    playTongueOutAnimation();
                        break;
                    case 12:
                    playTongueOutAnimation();
                        break;
                    case 13:
                    playTongueOutAnimation();
                        break;
                    case 14:
                    playTongueOutAnimation();
                        break;
                    case 15:
                    playTongueOutAnimation();
                        break;
                    case 16:
                    playTongueOutAnimation();
                        break;
                    case 17:
                        playTongueOutAnimation();
                        break;
                    case 18:
                        playFunnyEyesToNormalAnimation();
                        break;
                    case 19:
                        playIdleToSadAnimation();
                        break;
                    case 20:
                        playTearAnimation();
                        break;
                    case 21:
                        playTearAnimation();
                        break;
                    case 22:
                        playTearAnimation();
                        break;
                    case 23:
                        playTearAnimation();
                        break;
                    case 24:
                        playSadToIdleAnimation();
                        break;
                    case 25: 
                        playIdleToAngryAnimation();
                        break;
                    case 26:
                        playAngryToIdleAnimation();
                        break;
                    case 27:
                        playIdleToHappyAnimation();
                        break;
                    case 28:
                        playHappyToIdleAnimation();
                        break;
                    case 29:
                    playIdleToSadAnimation();
                    break;
                    case 30:
                    playSadToCryAnimation();
                    break;
                    case 32:
                    playCryingAnimation();
                    break;
                    case 33:
                    playCryingAnimation();
                    break;
                    case 34:
                    playCryingAnimation();
                    break;
                    case 35:
                    playCryingAnimation();
                    break;
                    case 36:
                    playCryingAnimation();
                    break;
                    case 37:
                    playCryingAnimation();
                    break;
                    case 38:
                    playCryToSadAnimation();
                    break;
                    case 39:
                    playSadToIdleAnimation();
                    break;
                    default:
                        animationIndex = -1; // Reset to start
                        break;
                }
                
                animationIndex++;
                if (animationIndex > 39) {
                    animationIndex = 0; // Loop back to start
                }
                
                lastAnimationTime = currentTime;
            }
            break;
        }
            
        case MODE_WEATHER: {
            // Weather mode - fetch and display weather
            static unsigned long lastWeatherUpdate = 0;
            static unsigned long lastWeatherError = 0;
            if (WiFi.status() == WL_CONNECTED) {
                bleSerialPrint("get weather data");
                
                if (millis() - lastWeatherUpdate > 60000) { // Update every 60 seconds
                    getWeatherData();
                    lastWeatherUpdate = millis();
                }
            } else {
                // Only show error message every 10 seconds to avoid blocking
                if (millis() - lastWeatherError > 10000) {
                    display_text("WiFi not connected\nCannot fetch weather");
                    bleSerialPrint("get weather data failed");
                    delay(2000);
                    lastWeatherError = millis();
                }
            }
            break;
        }
            
        case MODE_GAME: {
            // Game mode - run dino game
            bool touchPressed = digitalRead(TOUCH_SENSOR_PIN) == HIGH;
            runDinoGame(touchPressed);
            break;
        }
    }    
}
