/*
 * ardulumen - LED Control for ESP32
 * Created by autinerd and margau
 */

#define IfEffect(effect,class,...) if (effectType == effect) {animation->addEffect(new class(animation, __VA_ARGS__));}

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
  #include "ESPAsyncUDP.h"  
#endif

// LED-specific libarys
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "src/pixelpp/PixelPP.hpp"
#include "src/pixelpp/FillEffect.hpp"
#include "src/pixelpp/SineEffect.hpp"
#include "src/pixelpp/PixEffect.hpp"

// Initialize Objects
// Preferences prefs;
// HTTPClient* client = new HTTPClient();
AsyncUDP udp;
StaticJsonDocument<1024> json;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, 2, NEO_GRB + NEO_KHZ800);
PixelPP* animation = new PixelPP(strip.numPixels(), strip.getPixels(), LEDColor::GRB);
uint8_t incomingPacket[1024];

// Some constants
#define VERSION "0.0.1-dev"

// Timing
unsigned long last_frame = 0;
const unsigned long frame_delay = 20;
unsigned long last_poll = 0UL;
const unsigned int polling_delay = 1000U;
unsigned long now = 0;
int16_t currentSequence = -1;
int16_t currentInstance = -1;

void setup()
{
	// Boot and init debug
	Serial.begin(115200);
	Serial.print("ardulumen v");
	Serial.println(VERSION);
	animation->addEffect(new FillEffect(animation, {127,0,0}))
			 ->addEffect(new SineEffect(animation, 25, 2000))
			 ->addEffect(new PixEffect(animation, {0, 0, 255}, 500, 1));

	// setup Wifi
	WiFi.begin("ardulumen", emptyString);

	// client->setTimeout(1000);

	strip.begin();
	strip.setPixelColor(0,0,255,0);
	strip.show();
	delay(5000);
	if (WiFi.isConnected())
	{
		Serial.println("WiFi connected!");
	}
	else
	{
		Serial.println("WiFi not connected!");
	}
  // Listen on UDP Socket with callback
  if(udp.listen(3333)) {
    Serial.println("Listening on Port 3333");
        udp.onPacket([](AsyncUDPPacket packet) {
          Serial.println("Received data");
          size_t len = packet.length();
          strcpy((char*)incomingPacket, (char*)packet.data());
          deserializeJson(json, incomingPacket);
          analyzeRecievedJson();
        });
   } else {
      Serial.println("Failure Listening to UDP");
   }
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
 delay(1);
}
struct rgb ColorToRGB(uint32_t c) {
  rgb color = {0,0,0};
  color.red = ((c >> 16) & 0xFF);
  color.green = ((c >> 8) & 0xFF);
  color.blue = ((c) & 0xFF);
  return color;
}
void analyzeRecievedJson()
{
	if (currentInstance == -1)
	{
		currentInstance = json["instance"].as<int16_t>();
	}
  // Reset Sequence if master restarted
  if(json["runtime"].as<int16_t>() < (now - last_frame)) {
    currentSequence = 0;  
  }
	if (json["serial"].as<int16_t>() <= currentSequence || json["instance"].as<int16_t>() != currentInstance)
	{
		return;
	}
	currentSequence = json["serial"].as<int16_t>();
	Serial.println("JSON accepted!");
	animation->clearEffects();
	JsonArray effects = json["effects"];
	for (int8_t i = 0; i < effects.size(); i++)
	{
		JsonObject effect = effects[i];
		String effectType = effect["type"].as<String>();
		Serial.println("Found effect: " + effectType);
		/*IfEffect("fill", FillEffect, ColorToRGB(effect["color"].as<uint32_t>()))
		else IfEffect("sine", SineEffect, effect["w"].as<uint8_t>(), effect["p"].as<uint16_t>())
		else IfEffect("pix", PixEffect, ColorToRGB(effect["color"].as<uint32_t>()), effect["f"].as<uint16_t>(), effect["c"].as<uint8_t>())*/
    if(effectType=="fill") {
      Serial.println("Fill Effect");
      animation->addEffect(new FillEffect(animation, ColorToRGB(effect["color"].as<uint32_t>())));
    } else if(effectType=="sine") {
      Serial.println("Sine Effect");
      animation->addEffect(new SineEffect(animation, effect["w"].as<uint8_t>(), effect["p"].as<uint16_t>()));
    } else if(effectType=="pix") {
      Serial.println("Pix Effect");
      animation->addEffect(new PixEffect(animation, ColorToRGB(effect["color"].as<uint32_t>()), effect["f"].as<uint16_t>(), effect["c"].as<uint8_t>()));
    }
	}
}
