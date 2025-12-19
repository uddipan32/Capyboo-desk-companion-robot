# Reducing Sketch Size for ESP32

## Problem

The sketch uses 107% of available program storage space (1,415,204 bytes / 1,310,720 bytes).

## Solution 1: Change Partition Scheme (Recommended - Quick Fix)

In Arduino IDE:

1. Go to **Tools → Partition Scheme**
2. Select **"Huge APP (3MB No OTA/1MB SPIFFS)"** or **"No OTA (2MB APP/2MB SPIFFS)"**
3. This will give you more program space (up to 3MB instead of ~1.3MB)

**Note:** This disables OTA (Over-The-Air) updates, but gives you much more program space.

## Solution 2: Remove Test Animations from Loop

The `loop()` function has many test animations that aren't needed. Remove or comment them out for production.

## Solution 3: Use SPIFFS/LittleFS for Animations

Move animation data to external flash storage (SPIFFS/LittleFS) instead of storing in program memory. This requires:

- Formatting the flash with SPIFFS
- Uploading animation files separately
- Loading animations from flash at runtime

## Solution 4: Compress Animation Data

Use RLE (Run-Length Encoding) or other compression for bitmap data. This requires modifying how animations are stored and loaded.

## Recommended Approach

1. **Immediate fix:** Change partition scheme to "Huge APP"
2. **Clean up:** Remove test animations from loop()
3. **Future optimization:** Consider moving animations to SPIFFS if you need OTA updates
