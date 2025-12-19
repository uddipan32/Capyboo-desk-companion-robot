#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Display object (shared across headers) - define it here
Adafruit_SH1106G display(128,64,&Wire,-1);


void display_bitmap(const unsigned char* frame) {
    display.clearDisplay();
    display.drawBitmap(0, 0, frame, 128, 64, SH110X_WHITE);
    display.display();
}

void display_text(const char* text) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    
    // Center the text
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (128 - w) / 2;
    int16_t y = (64 - h) / 2;
    display.setCursor(x, y);
    display.println(text);
    display.display();
}