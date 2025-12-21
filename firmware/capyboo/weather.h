#include "wifi.h"  // Use wifi.h for WiFi connection management
#include <ArduinoJson.h>
#include <HTTPClient.h>  // For ESP32 boards
#include "secrets.h"

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <Wire.h> // library requires for IIC communication

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define I2C_ADDRESS 0x3C  // I2C address for SH1106 (usually 0x3C or 0x3D)

// Initialize OLED display (extern - defined in main file)
extern Adafruit_SH1106G display;

// OpenWeatherMap API - GET YOUR FREE API KEY FROM https://openweathermap.org/api
const String apiKey = WEATHER_API_KEY;
const String city = "Guwahati";  // Change to your city
const String countryCode = "IN";  // Change to your country code

// API endpoint
String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&appid=" + apiKey + "&units=metric";

unsigned long lastTime = 0;
unsigned long timerDelay = 60000;  // Update every 1 minute (60000 ms = 1 * 60 * 1000)

// Forward declaration
void displayWeatherOnOLED(String city, float temp, float feelsLike, int humidity, String desc);

void getWeatherData() {
    if (WiFi.status() != WL_CONNECTED) {
        // request bluetooth to get the weather data
        bleSerialPrintln("Please connect to WiFi to get weather data");
        return;
    }
    HTTPClient http;
    
    // Show cool loading animation
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(25, 25);
    display.println("Fetching data");
    
    // Animated loading dots
    for (int i = 0; i < 3; i++) {
        display.fillRect(75 + i * 8, 25, 4, 4, SH110X_WHITE);
    }
    display.display();
    
    http.begin(serverPath);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        
        String payload = http.getString();
        
        // Parse JSON response
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        
        // Extract weather data
        float temperature = doc["main"]["temp"];
        float feelsLike = doc["main"]["feels_like"];
        int humidity = doc["main"]["humidity"];
        String description = doc["weather"][0]["description"];
        String cityName = doc["name"];
        String country = doc["sys"]["country"];
        
        // Display weather information on Serial
        Serial.println("\n=================================");
        Serial.print("Location: ");
        Serial.print(cityName);
        Serial.print(", ");
        Serial.println(country);
        Serial.println("---------------------------------");
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");
        Serial.print("Feels like: ");
        Serial.print(feelsLike);
        Serial.println(" °C");
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
        Serial.print("Weather: ");
        Serial.println(description);
        Serial.println("=================================\n");
    
    // Display weather information on OLED
    displayWeatherOnOLED(cityName, temperature, feelsLike, humidity, description);
    
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println("Failed to get weather data. Check your API key and network connection.");
    
    // Show error with design
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, SH110X_WHITE);
    display.fillRect(54, 10, 20, 20, SH110X_WHITE);
    display.fillRect(58, 14, 12, 12, SH110X_BLACK);
    display.setTextSize(1);
    display.setCursor(40, 8);
    display.println("ERROR!");
    display.setCursor(20, 35);
    display.print("Code: ");
    display.println(httpResponseCode);
    display.setCursor(15, 50);
    display.println("Check API key");
    display.display();
  }
  
  http.end();
}

void displayWeatherOnOLED(String city, float temp, float feelsLike, int humidity, String desc) {
  display.clearDisplay();
  
  // Draw border
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);
  display.drawLine(0, 12, 128, 12, SH110X_WHITE);
  
  // City name (top, centered)
  display.setTextSize(1);
  display.setCursor(64 - (city.length() * 3), 2);
  display.print(city);
  
  // Temperature (large, left side with icon)
  display.setTextSize(3);
  display.setCursor(5, 18);
  display.print(temp, 0);
  
  // Degree symbol and unit
  display.setTextSize(1);
  display.setCursor(65, 20);
  display.print("o");
  display.setCursor(70, 18);
  display.setTextSize(2);
  display.print("C");
  
  // Feels like (right side, compact)
  display.setTextSize(1);
  display.setCursor(85, 18);
  display.print("Feels");
  display.setCursor(85, 28);
  display.print(feelsLike, 0);
  display.print("o");
  
  // Humidity with icon (bottom left)
  display.setCursor(5, 48);
  display.print("H:");
  display.print(humidity);
  display.print("%");
  
  // Weather description (bottom right, truncated if needed)
  display.setCursor(50, 48);
  String shortDesc = desc;
  if (shortDesc.length() > 12) {
    shortDesc = shortDesc.substring(0, 9) + "...";
  }
  // Capitalize first letter
  if (shortDesc.length() > 0) {
    shortDesc = String((char)toupper(shortDesc[0])) + shortDesc.substring(1);
  }
  display.print(shortDesc);
  
  // Draw separator line
  display.drawLine(0, 42, 128, 42, SH110X_WHITE);
  
  display.display();
}
