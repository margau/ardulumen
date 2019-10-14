/*
 * ardulumen - LED Control for ESP32
 * Created by autinerd and margau
 */

// Libarys for basic functions
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

// Initialize Objects
Preferences prefs;

// Some constants
#define VERSION "0.0.1-dev"

void setup() {
  // Boot and init debug
  Serial.begin(115200);
  Serial.print("ardulumen v");
  Serial.println(VERSION);
  // Initialize Preferences
  prefs.begin("ardulumen");
  // Initialize WiFi AP
  char apName[30];
  prefs.getString("apName","ardulumen").toCharArray(apName, 50);
  Serial.print("Create AP with SSID "); Serial.println(apName);
  WiFi.softAP(apName);
}

void loop() {
  
}
