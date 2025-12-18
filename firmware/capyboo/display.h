#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Display object (shared across headers) - define it here
Adafruit_SH1106G display(128,64,&Wire,-1);


void display_bitmap(const unsigned char* frame) {
    display.clearDisplay();
    display.drawBitmap(0, 0, frame, 128, 64, SH110X_WHITE);
    display.display();
}