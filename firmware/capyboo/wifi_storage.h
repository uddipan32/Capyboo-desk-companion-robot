#ifndef WIFI_STORAGE_H
#define WIFI_STORAGE_H

#include <Preferences.h>

Preferences preferences;

// Keys for storing WiFi credentials
const char* WIFI_SSID_KEY = "wifi_ssid";
const char* WIFI_PASSWORD_KEY = "wifi_pass";
const char* WIFI_CONFIGURED_KEY = "wifi_cfg";

// Initialize WiFi storage
void initWiFiStorage() {
    preferences.begin("wifi", false); // false = read/write mode
}

// Save WiFi credentials to non-volatile storage
bool saveWiFiCredentials(const char* ssid, const char* password) {
    if (ssid == NULL || password == NULL || strlen(ssid) == 0) {
        return false;
    }
    
    preferences.putString(WIFI_SSID_KEY, ssid);
    preferences.putString(WIFI_PASSWORD_KEY, password);
    preferences.putBool(WIFI_CONFIGURED_KEY, true);
    
    Serial.print("Saved WiFi credentials: SSID=");
    Serial.println(ssid);
    return true;
}

// Load WiFi credentials from non-volatile storage
bool loadWiFiCredentials(char* ssid, char* password, size_t maxLen) {
    if (!preferences.getBool(WIFI_CONFIGURED_KEY, false)) {
        Serial.println("No WiFi credentials stored");
        return false;
    }
    
    String ssidStr = preferences.getString(WIFI_SSID_KEY, "");
    String passStr = preferences.getString(WIFI_PASSWORD_KEY, "");
    
    if (ssidStr.length() == 0) {
        return false;
    }
    
    // Copy to provided buffers
    ssidStr.toCharArray(ssid, maxLen);
    passStr.toCharArray(password, maxLen);
    
    Serial.print("Loaded WiFi credentials: SSID=");
    Serial.println(ssid);
    return true;
}

// Check if WiFi credentials are stored
bool hasStoredWiFiCredentials() {
    return preferences.getBool(WIFI_CONFIGURED_KEY, false);
}

// Clear stored WiFi credentials
void clearWiFiCredentials() {
    preferences.remove(WIFI_SSID_KEY);
    preferences.remove(WIFI_PASSWORD_KEY);
    preferences.remove(WIFI_CONFIGURED_KEY);
    Serial.println("WiFi credentials cleared");
}

// Close preferences (call when done)
void closeWiFiStorage() {
    preferences.end();
}

#endif // WIFI_STORAGE_H

