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
#elif defined(ESP8266)
// ESP8266 specific
	#include <ESP8266WiFi.h>
#endif

#include <WiFiClient.h>

// LED-specific libarys
#include <Adafruit_NeoPixel.h>
#include "src/pixelpp/PixelPP.h"
#include "src/pixelpp/FillEffect.hpp"
#include "src/pixelpp/SineEffect.hpp"
#include "src/pixelpp/PixEffect.hpp"

// Initialize Objects
// Preferences prefs;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, 2, NEO_GRB + NEO_KHZ800);
PixelPP* animation = new PixelPP(65, strip.getPixels(), LEDColor::GRB);

// Some constants
#define VERSION "0.0.1-dev"

// Timing
unsigned long last_frame = 0;
const unsigned long frame_delay = 20;
unsigned long now = 0;

void setup()
{
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
	animation->addEffect(new FillEffect(animation, (rgb){127, 0, 0}))
        ->addEffect(new SineEffect(animation, 25, 2000))
	      ->addEffect(new PixEffect(animation, (rgb){0, 0, 255}, 500, 1));
  
	//  WiFi.softAP(apName);
	strip.begin();
	strip.setPixelColor(0,0,255,0);
	strip.show();
	delay(2000);
}

void loop()
{
	now = millis();
	if ((now - last_frame) >= frame_delay)
	{
		last_frame = now;
		animation->render();
		strip.show();
	}
}
