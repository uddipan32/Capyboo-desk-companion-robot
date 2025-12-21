#ifndef DISPLAY_H
#define DISPLAY_H

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
    
    // Calculate how many characters fit per line (128 pixels / ~6 pixels per char)
    const int charsPerLine = 21;  // Approximately 21 characters per line at size 1
    const int maxLines = 8;        // Maximum lines that fit (64 pixels / 8 pixels per line)
    
    String textStr = String(text);
    int textLen = textStr.length();
    
    // If text fits on one line, center it
    if (textLen <= charsPerLine) {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        int16_t x = (128 - w) / 2;
        int16_t y = (64 - h) / 2;
        display.setCursor(x, y);
        display.println(text);
    } else {
        // Text is too long - wrap to multiple lines
        int line = 0;
        int pos = 0;
        
        while (pos < textLen && line < maxLines) {
            int endPos = pos + charsPerLine;
            if (endPos > textLen) {
                endPos = textLen;
            }
            
            // Extract line
            String lineText = textStr.substring(pos, endPos);
            
            // Display line (left-aligned for multi-line)
            display.setCursor(0, line * 8);  // 8 pixels per line at size 1
            display.println(lineText);
            
            pos = endPos;
            line++;
        }
        
        // If text was truncated, show ellipsis on last line
        if (pos < textLen && line >= maxLines) {
            display.setCursor(0, (maxLines - 1) * 8);
            String lastLine = textStr.substring((maxLines - 1) * charsPerLine, (maxLines - 1) * charsPerLine + charsPerLine - 3);
            display.print(lastLine);
            display.print("...");
        }
    }
    
    display.display();
}

#endif // DISPLAY_H