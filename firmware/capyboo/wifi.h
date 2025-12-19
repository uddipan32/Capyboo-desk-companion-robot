#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>

// Connect to WiFi
bool connectWifi(const char* ssid, const char* password) {
    if (ssid == NULL || strlen(ssid) == 0) {
        Serial.println("Invalid WiFi SSID");
        return false;
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(); // Disconnect any existing connection
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println();
        Serial.println("WiFi connection failed!");
        return false;
    }
}

// Disconnect WiFi
void disconnectWifi() {
    WiFi.disconnect();
    Serial.println("WiFi disconnected");
}

#endif // WIFI_H