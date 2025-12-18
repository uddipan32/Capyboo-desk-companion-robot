#include "bluetooth.h"
#include "display.h"
#include <Wire.h>

#include "face_animation.h"


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

    // Play wakeup animation once at startup
    playWakeupAnimation();
}

void loop() {
    // Handle BLE connection/disconnection (required for BLE communication)
    handleBLESerial();
}

