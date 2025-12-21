#include "bluetooth.h"
#include "display.h"
#include <Wire.h>

#include "face_animation.h"
#include "weather.h"
#include "dino_game.h"
#include "clock.h"

// Touch sensor pin (from definitions.h)
const int TOUCH_SENSOR_PIN = 4;

// Mode definitions
enum Mode {
    MODE_ANIMATION,
    MODE_WEATHER,
    MODE_GAME,
    MODE_CLOCK
};
String message = "";
String currentCity = "";
float currentTemperature = 0;
float currentFeelsLike = 0;
int currentHumidity = 0;
String currentDescription = "";
int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;

Mode currentMode = MODE_ANIMATION;  // Default mode


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
        case MODE_CLOCK:
            modeText += "Clock";
            break;
    }
    display_text(modeText.c_str());
    delay(2000);
}

// Animation sequence state
int animationIndex = 0;
unsigned long lastAnimationTime = 0;
const unsigned long ANIMATION_DELAY = 1000; // Delay between animations in ms

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
        // Handle mode switching commands
        if (lowerCommand.startsWith("mode:")) {
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
            } else if (modeStr == "clock") {
                currentMode = MODE_CLOCK;
                bleSerialPrintln("Switched to Clock mode");
                displayCurrentMode();
            } else {
                String errorMsg = "Unknown mode: " + modeStr;
                display_text(errorMsg.c_str());
                bleSerialPrintln("Unknown mode. Use: mode:animation, mode:weather, or mode:game");
                delay(2000);
            }
        } else if (lowerCommand.startsWith("weather:")) {
            Serial.print("Weather command: ");
            String weatherCommand = lowerCommand.substring(8); // Get text after "weather:"
            weatherCommand.trim();
            Serial.print("Parsing: ");
            Serial.println(weatherCommand);
            // command format: weather:city:temperature:feels_like:humidity:description
            // Parse the colon-separated values
            int pos = 0;
            int nextPos = weatherCommand.indexOf(":", pos);
            
            // Parse city (required)
            if (nextPos > 0) {
                currentCity = weatherCommand.substring(pos, nextPos);
                pos = nextPos + 1;
                Serial.print("City: ");
                Serial.println(currentCity);
            } else if (nextPos == 0) {
                // Colon at start - empty city
                bleSerialPrintln("Invalid weather format: city cannot be empty");
            } else {
                // No colon found - city is the entire string
                currentCity = weatherCommand;
                pos = weatherCommand.length();
                Serial.print("City (no more fields): ");
                Serial.println(currentCity);
            }
            
            // Parse temperature (required)
            if (pos < weatherCommand.length()) {
                nextPos = weatherCommand.indexOf(":", pos);
                if (nextPos > pos) {
                    currentTemperature = weatherCommand.substring(pos, nextPos).toFloat();
                    pos = nextPos + 1;
                    Serial.print("Temperature: ");
                    Serial.println(currentTemperature);
                } else if (nextPos == -1) {
                    // No more colons - temperature is the rest
                    currentTemperature = weatherCommand.substring(pos).toFloat();
                    pos = weatherCommand.length();
                    Serial.print("Temperature (last field): ");
                    Serial.println(currentTemperature);
                } else {
                    bleSerialPrintln("Invalid weather format: missing temperature");
                    pos = weatherCommand.length(); // Skip rest
                }
            }
            
            // Parse feels_like (required)
            if (pos < weatherCommand.length()) {
                nextPos = weatherCommand.indexOf(":", pos);
                if (nextPos > pos) {
                    currentFeelsLike = weatherCommand.substring(pos, nextPos).toFloat();
                    pos = nextPos + 1;
                    Serial.print("Feels like: ");
                    Serial.println(currentFeelsLike);
                } else if (nextPos == -1) {
                    // No more colons - feels_like is the rest
                    currentFeelsLike = weatherCommand.substring(pos).toFloat();
                    pos = weatherCommand.length();
                    Serial.print("Feels like (last field): ");
                    Serial.println(currentFeelsLike);
                } else {
                    bleSerialPrintln("Invalid weather format: missing feels_like");
                    pos = weatherCommand.length(); // Skip rest
                }
            }
            
            // Parse humidity (required, but description is optional)
            if (pos < weatherCommand.length()) {
                nextPos = weatherCommand.indexOf(":", pos);
                if (nextPos > pos) {
                    // Has description after humidity
                    currentHumidity = weatherCommand.substring(pos, nextPos).toInt();
                    pos = nextPos + 1;
                    currentDescription = weatherCommand.substring(pos);
                    Serial.print("Humidity: ");
                    Serial.println(currentHumidity);
                    Serial.print("Description: ");
                    Serial.println(currentDescription);
                } else if (nextPos == -1) {
                    // No description - humidity is the rest
                    currentHumidity = weatherCommand.substring(pos).toInt();
                    currentDescription = "";
                    Serial.print("Humidity (last field): ");
                    Serial.println(currentHumidity);
                } else {
                    bleSerialPrintln("Invalid weather format: missing humidity");
                }
                
                // Display the weather data
                displayWeatherOnOLED(currentCity, currentTemperature, currentFeelsLike, currentHumidity, currentDescription);
                bleSerialPrintln("Weather data updated");
            } else {
                bleSerialPrintln("Invalid weather format: missing humidity");
            }
        }
        else if (lowerCommand.startsWith("message:")) {
            message = lowerCommand.substring(8); // Get text after "message:"
            message.trim();
        } else if (lowerCommand.startsWith("time:")) {
            String clockCommand = lowerCommand.substring(5); // Get text after "time:"
            clockCommand.trim();

            // command format: clock:HH:MM:SS or clock:HH:MM:SS DD/MM/YYYY
            // Use the setTimeFromString function from clock.h
            if (setTimeFromString(clockCommand)) {
                bleSerialPrintln("Clock time set successfully");
                display_text("Time set");
                delay(2000);
            } else {
                bleSerialPrintln("Invalid clock format. Use: clock:HH:MM:SS or clock:HH:MM:SS DD/MM/YYYY");
            }
        }

    }

   

    // if touch sensor is pressed, play tickle animation
    if (digitalRead(TOUCH_SENSOR_PIN) == HIGH && currentMode != MODE_GAME) {
        playTickleStartAnimation();
        playTickleAnimation();
        playTickleAnimation();
        playTickleEndAnimation();
        message = "";
    }

    if (message.length() > 0) {
        display_text(message.c_str());
        delay(500);
        return;
    }

    // Handle different modes
    switch (currentMode) {
        case MODE_ANIMATION: {
            // playTickleStartAnimation();
            // playTickleAnimation();
            // playTickleAnimation();
            // playTickleAnimation();
            // playTickleAnimation();
            // playTickleAnimation();
            // playTickleAnimation();
            // playTickleEndAnimation();
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
            displayWeatherOnOLED(currentCity, currentTemperature, currentFeelsLike, currentHumidity, currentDescription);
            break;
        }
            
        case MODE_GAME: {
            // Game mode - run dino game
            bool touchPressed = digitalRead(TOUCH_SENSOR_PIN) == HIGH;
            runDinoGame(touchPressed);
            break;
        } 
        case MODE_CLOCK: {
            // Clock mode - display clock
            updateClock();

            break;
        }
    }    
}
