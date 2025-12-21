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
        // Text is too long - wrap to multiple lines with word boundary awareness
        int line = 0;
        int pos = 0;
        bool textTruncated = false;
        
        while (pos < textLen && line < maxLines) {
            int remainingChars = textLen - pos;
            int lineEndPos;
            
            if (remainingChars <= charsPerLine) {
                // Remaining text fits on one line
                lineEndPos = textLen;
            } else {
                // Find the best break point (prefer space, but break at char limit if needed)
                lineEndPos = pos + charsPerLine;
                
                // Look for a space before the character limit (prefer word boundary)
                int spacePos = textStr.lastIndexOf(' ', lineEndPos);
                if (spacePos > pos) {
                    // Found a space - break there
                    lineEndPos = spacePos;
                }
                // If no space found or word is too long, break at character limit
            }
            
            // Check if this is the last line and text will be truncated
            if (line == maxLines - 1 && lineEndPos < textLen) {
                // Last line - make room for ellipsis
                if (lineEndPos - pos > charsPerLine - 3) {
                    lineEndPos = pos + charsPerLine - 3;
                }
                textTruncated = true;
            }
            
            // Extract line text
            String lineText = textStr.substring(pos, lineEndPos);
            
            // Display line (left-aligned for multi-line)
            display.setCursor(0, line * 8);  // 8 pixels per line at size 1
            display.print(lineText);
            
            // Add ellipsis if text was truncated
            if (textTruncated && line == maxLines - 1) {
                display.print("...");
            } else {
                display.println(); // Move to next line
            }
            
            // Move to next position (skip space if we broke at a word boundary)
            pos = lineEndPos;
            if (pos < textLen && textStr.charAt(pos) == ' ') {
                pos++; // Skip the space at the start of next line
            }
            
            line++;
            
            // Stop if we've displayed all text or reached max lines
            if (pos >= textLen || line >= maxLines) {
                break;
            }
        }
    }
    
    display.display();
}

#endif // DISPLAY_H