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
	#include <ESP8266HTTPClient.h>
#endif

// LED-specific libarys
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "src/pixelpp/PixelPP.h"
#include "src/pixelpp/FillEffect.hpp"
#include "src/pixelpp/SineEffect.hpp"
#include "src/pixelpp/PixEffect.hpp"

HTTPClient client;
StaticJsonDocument<1024> json;

// Initialize Objects
// Preferences prefs;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, 2, NEO_GRB + NEO_KHZ800);
PixelPP* animation = new PixelPP(65, strip.getPixels(), LEDColor::GRB);

// Some constants
#define VERSION "0.0.1-dev"

// Timing
unsigned long last_frame = 0;
const unsigned long frame_delay = 20;
unsigned long last_poll = 0UL;
const unsigned int polling_delay = 1000U;
unsigned long now = 0;

void setup()
{
	// Boot and init debug
	Serial.begin(115200);
	Serial.print("ardulumen v");
	Serial.println(VERSION);
	animation->addEffect(new FillEffect(animation, (rgb){127, 0, 0}))
			 ->addEffect(new SineEffect(animation, 25, 2000))
			 ->addEffect(new PixEffect(animation, (rgb){0, 0, 255}, 500, 1));

	// setup Wifi
	WiFi.begin("ardulumen", emptyString);

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
	if ((now - last_poll) >= polling_delay)
	{
		last_poll = now;
		if (client.begin("http://" + WiFi.gatewayIP().toString() + "/led"))
		{
			int httpCode = client.GET();
			if (httpCode == HTTP_CODE_OK)
			{
				deserializeJson(json, client.getString());
				Serial.println(json["instance"].as<uint8_t>());

				// {"instance":0,"serial":0,"effects":[{"type":"fill","color":16711680},{"type":"sine","w":25,"p":2000},{"type":"pix","color":16711680,"f":200,"c":1}]}
			}
			client.end();
		}
	}
}
