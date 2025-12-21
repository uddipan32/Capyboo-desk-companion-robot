#ifndef CLOCK_H
#define CLOCK_H

#include <time.h>
#include "display.h"

// Time structure
struct ClockTime {
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
    String dayOfWeek;
};

// Global time variables
bool timeInitialized = false;
unsigned long lastTimeUpdate = 0;
unsigned long timeSetMillis = 0; // When time was set
time_t timeSetEpoch = 0; // Epoch time when set

const unsigned long TIME_UPDATE_INTERVAL = 1000; // Update display every second

// Day names
const char* DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* MONTH_NAMES[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Days in each month (non-leap year)
const int DAYS_IN_MONTH[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Calculate day of week (Zeller's congruence)
int calculateDayOfWeek(int day, int month, int year) {
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int k = year % 100;
    int j = year / 100;
    int h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    // Convert to 0=Sunday, 1=Monday, etc.
    return ((h + 5) % 7);
}

// Check if year is leap year
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Initialize clock with manual time
// Format: setTime(hour, minute, second, day, month, year)
bool setTime(int hour, int minute, int second, int day, int month, int year) {
    // Validate inputs
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || 
        second < 0 || second > 59 || day < 1 || day > 31 || 
        month < 1 || month > 12 || year < 2020 || year > 2099) {
        Serial.println("Invalid time values");
        return false;
    }
    
    // Validate day for month
    int maxDays = DAYS_IN_MONTH[month - 1];
    if (month == 2 && isLeapYear(year)) {
        maxDays = 29;
    }
    if (day > maxDays) {
        Serial.println("Invalid day for month");
        return false;
    }
    
    // Create tm structure
    struct tm timeinfo;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month - 1; // tm_mon is 0-11
    timeinfo.tm_year = year - 1900; // tm_year is years since 1900
    timeinfo.tm_wday = calculateDayOfWeek(day, month, year);
    timeinfo.tm_isdst = 0; // No daylight saving
    
    // Set system time
    timeSetEpoch = mktime(&timeinfo);
    timeSetMillis = millis();
    timeInitialized = true;
    
    Serial.print("Time set to: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.print(second);
    Serial.print(" ");
    Serial.print(day);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.println(year);
    
    return true;
}

// Set time from string format: "HH:MM:SS DD/MM/YYYY" or "HH:MM:SS"
bool setTimeFromString(String timeStr) {
    timeStr.trim();
    
    // Try format: "HH:MM:SS DD/MM/YYYY"
    int timeEnd = timeStr.indexOf(" ");
    String timePart, datePart;
    
    if (timeEnd > 0) {
        timePart = timeStr.substring(0, timeEnd);
        datePart = timeStr.substring(timeEnd + 1);
    } else {
        // Only time provided, use current date or default
        timePart = timeStr;
        datePart = "";
    }
    
    // Parse time: HH:MM:SS
    int colon1 = timePart.indexOf(":");
    int colon2 = timePart.indexOf(":", colon1 + 1);
    
    if (colon1 < 0 || colon2 < 0) {
        Serial.println("Invalid time format. Use HH:MM:SS");
        return false;
    }
    
    int hour = timePart.substring(0, colon1).toInt();
    int minute = timePart.substring(colon1 + 1, colon2).toInt();
    int second = timePart.substring(colon2 + 1).toInt();
    
    // Parse date: DD/MM/YYYY (optional)
    int day = 1, month = 1, year = 2024;
    if (datePart.length() > 0) {
        int slash1 = datePart.indexOf("/");
        int slash2 = datePart.indexOf("/", slash1 + 1);
        
        if (slash1 > 0 && slash2 > 0) {
            day = datePart.substring(0, slash1).toInt();
            month = datePart.substring(slash1 + 1, slash2).toInt();
            year = datePart.substring(slash2 + 1).toInt();
        }
    }
    
    return setTime(hour, minute, second, day, month, year);
}

// Get current time
ClockTime getCurrentTime() {
    ClockTime clockTime;
    
    if (!timeInitialized) {
        // Return default time if not set
        clockTime.hour = 0;
        clockTime.minute = 0;
        clockTime.second = 0;
        clockTime.day = 1;
        clockTime.month = 1;
        clockTime.year = 2024;
        clockTime.dayOfWeek = "---";
        return clockTime;
    }
    
    // Calculate elapsed time since time was set
    unsigned long elapsedMillis = millis() - timeSetMillis;
    time_t currentEpoch = timeSetEpoch + (elapsedMillis / 1000);
    
    // Convert to tm structure
    struct tm* timeinfo = localtime(&currentEpoch);
    
    clockTime.hour = timeinfo->tm_hour;
    clockTime.minute = timeinfo->tm_min;
    clockTime.second = timeinfo->tm_sec;
    clockTime.day = timeinfo->tm_mday;
    clockTime.month = timeinfo->tm_mon + 1; // tm_mon is 0-11
    clockTime.year = timeinfo->tm_year + 1900; // tm_year is years since 1900
    clockTime.dayOfWeek = String(DAY_NAMES[timeinfo->tm_wday]);
    
    return clockTime;
}

// Format time as HH:MM:SS
String formatTime(ClockTime t, bool includeSeconds = true) {
    String timeStr = "";
    if (t.hour < 10) timeStr += "0";
    timeStr += String(t.hour);
    timeStr += ":";
    if (t.minute < 10) timeStr += "0";
    timeStr += String(t.minute);
    if (includeSeconds) {
        timeStr += ":";
        if (t.second < 10) timeStr += "0";
        timeStr += String(t.second);
    }
    return timeStr;
}

// Format date as DD MMM YYYY
String formatDate(ClockTime t) {
    String dateStr = "";
    if (t.day < 10) dateStr += "0";
    dateStr += String(t.day);
    dateStr += " ";
    dateStr += String(MONTH_NAMES[t.month]);
    dateStr += " ";
    dateStr += String(t.year);
    return dateStr;
}


// Display compact clock (smaller format)
void displayCompactClock() {
    ClockTime t = getCurrentTime();
    
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    
    // Time (large)
    String timeStr = formatTime(t, false); // HH:MM without seconds
    display.setTextSize(3);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (128 - w) / 2;
    display.setCursor(x, 10);
    display.print(timeStr);
    
    // Seconds (small, below time)
    display.setTextSize(1);
    String secStr = "";
    if (t.second < 10) secStr += "0";
    secStr += String(t.second);
    display.getTextBounds(secStr, 0, 0, &x1, &y1, &w, &h);
    x = (128 - w) / 2;
    display.setCursor(x, 40);
    display.print(secStr);
    
    // Date (bottom)
    String dateStr = formatDate(t);
    display.getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
    x = (128 - w) / 2;
    display.setCursor(x, 52);
    display.print(dateStr);
    
    display.display();
}

// Update clock (call this in loop)
void updateClock() {
    // Update display every second
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate >= TIME_UPDATE_INTERVAL) {
        displayCompactClock();
        lastDisplayUpdate = millis();
    }
}

// Initialize clock (optional, sets default time)
bool initClock() {
    // Set default time if not already set
    if (!timeInitialized) {
        // Default: 00:00:00 01/01/2024
        return setTime(0, 0, 0, 1, 1, 2024);
    }
    return true;
}

#endif // CLOCK_H
