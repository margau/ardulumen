/*
 * ardulumen - LED Control for ESP32
 * Created by autinerd and margau
 */

// Libarys for basic functions
// ESP32 specific
#if defined(ESP32)
  #include <Preferences.h>
  #include <WiFiAP.h>
  #include <WiFi.h>
#endif
// ESP8266 specific
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <WiFiClient.h>

// LED-specific libarys
#include <Adafruit_NeoPixel.h>
#include "src/pixelpp/PixelPP.h"
#include "src/pixelpp/FillEffect.hpp"
#include "src/pixelpp/SineEffect.hpp"

// Initialize Objects
// Preferences prefs;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, 2, NEO_GRB + NEO_KHZ800);
PixelPP animation = PixelPP(16, COLOR_RGB, strip.getPixels());

// Some constants
#define VERSION "0.0.1-dev"

// Timing
unsigned long last_frame = 0;
unsigned long frame_delay = 40;
unsigned long now = 0;

void setup() {
  // Boot and init debug
  Serial.begin(115200);
  Serial.print("ardulumen v");
  Serial.println(VERSION);
  // Initialize Preferences
  // prefs.begin("ardulumen");
  // Initialize WiFi AP
  char apName[30] = "ardulumen";
  // prefs.getString("apName","ardulumen").toCharArray(apName, 50);
  Serial.print("Create AP with SSID "); Serial.println(apName);
  FillEffect effect_f = FillEffect(&animation, (rgb){255, 0, 0});
  SineEffect effect_s = SineEffect(&animation, 10);
  animation.addEffect(effect_f);
  animation.addEffect(effect_s);
//  WiFi.softAP(apName);

  strip.begin();
  strip.show();
}

void loop() {
  //strip.setPixelColor(0, strip.Color(255, 0, 0));
  //strip.show();

  now = millis();
  if((now - last_frame) >= frame_delay) {
    last_frame = now;
    animation.render();
    strip.show();
  }
}
