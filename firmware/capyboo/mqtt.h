#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "secrets.h"  // MQTT credentials

// MQTT Client object
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// MQTT Configuration - credentials loaded from secrets.h
const int MQTT_PORT = 8883;  // SSL port for HiveMQ Cloud

// MQTT Topics
const char* MQTT_TOPIC_SUBSCRIBE = "capyboo/commands";  // Topic to receive commands
const char* MQTT_TOPIC_PUBLISH = "capyboo/status";      // Topic to publish status

// MQTT connection state
bool mqttConnected = false;
String lastReceivedMessage = "";

// MQTT Callback function - called when a message is received
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    
    lastReceivedMessage = "";
    for (int i = 0; i < length; i++) {
        lastReceivedMessage += (char)payload[i];
    }
    Serial.println(lastReceivedMessage);
}

// Initialize MQTT connection
bool initMQTT() {
    // Set root CA certificate (optional but recommended for production)
    // For HiveMQ Cloud, you can use their certificate or skip for testing
    // espClient.setCACert(hivemq_root_ca);
    
    // For testing, you can skip certificate validation (not recommended for production)
    espClient.setInsecure();
    
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    Serial.println("MQTT client initialized");
    return true;
}

// Connect to MQTT broker
bool connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Cannot connect to MQTT.");
        return false;
    }
    
    if (mqttClient.connected()) {
        return true;
    }
    
    Serial.print("Connecting to MQTT broker: ");
    Serial.println(MQTT_BROKER);
    
    // Create a unique client ID
    String clientId = "Capyboo-ESP32-" + String(random(0xffff), HEX);
    
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println("Connected to MQTT broker!");
        mqttConnected = true;
        
        // Subscribe to command topic
        if (mqttClient.subscribe(MQTT_TOPIC_SUBSCRIBE)) {
            Serial.print("Subscribed to topic: ");
            Serial.println(MQTT_TOPIC_SUBSCRIBE);
        } else {
            Serial.println("Failed to subscribe to topic");
        }
        
        return true;
    } else {
        Serial.print("Failed to connect to MQTT broker. Error code: ");
        Serial.println(mqttClient.state());
        mqttConnected = false;
        return false;
    }
}

// Publish a message to MQTT
bool publishMQTT(const char* topic, const char* message) {
    if (!mqttClient.connected()) {
        if (!connectMQTT()) {
            return false;
        }
    }
    
    bool result = mqttClient.publish(topic, message);
    if (result) {
        Serial.print("Published to ");
        Serial.print(topic);
        Serial.print(": ");
        Serial.println(message);
    } else {
        Serial.println("Failed to publish message");
    }
    return result;
}

// Publish status message
bool publishStatus(const char* status) {
    return publishMQTT(MQTT_TOPIC_PUBLISH, status);
}

// Check if a message was received
bool mqttMessageAvailable() {
    return lastReceivedMessage.length() > 0;
}

// Get the last received message
String getMQTTMessage() {
    String msg = lastReceivedMessage;
    lastReceivedMessage = "";  // Clear after reading
    return msg;
}

// Handle MQTT connection (call this in loop)
void handleMQTT() {
    if (!mqttClient.connected()) {
        mqttConnected = false;
        // Try to reconnect every 5 seconds
        static unsigned long lastReconnectAttempt = 0;
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            connectMQTT();
        }
    } else {
        mqttClient.loop();  // Process incoming messages - this calls the callback
    }
}

#endif // MQTT_H

