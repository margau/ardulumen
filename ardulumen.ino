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
  #include "ESPAsyncUDP.h"
#endif

// LED-specific libarys
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "src/pixelpp/PixelPP.hpp"
#include "src/pixelpp/effects.hpp"
#include "FS.h"

// Initialize Objects
AsyncUDP udp;
StaticJsonDocument<1024> json;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, 2, NEO_GRB + NEO_KHZ800);
PixelPP* animation = new PixelPP(strip.numPixels(), strip.getPixels(), LEDColor::GRB);
uint8_t incomingPacket[1024];

WiFiEventHandler mConnectHandler;
WiFiEventHandler mDisconnectHandler;


// Some constants
#define VERSION "0.0.1-dev"
#define SEQUENCE_FILE_PATH "/sequence.json"
#define IfEffect(effect,class,...) if (effectType == effect) \
									{ \
									Serial.println(#class " recognized"); \
									animation->addEffect(new class(animation, __VA_ARGS__)); \
									}

// Timing
uint32_t last_frame = 0;
const uint32_t frame_delay = 20;
uint32_t last_poll = 0UL;
const uint32_t polling_delay = 1000U;
uint32_t now = 0;
int32_t currentSequence = -1;
int16_t currentInstance = -1;
File sequenceFile;
bool newPacket = false;

void setup()
{
	// Boot and init debug
	Serial.begin(115200);
	Serial.println("ardulumen v" VERSION);

	// setup Wifi
	WiFi.begin("ardulumen", emptyString);

	// setup SPIFFS
	SPIFFS.begin();
	if (SPIFFS.exists(SEQUENCE_FILE_PATH))
	{
		Serial.println("File found!");
		sequenceFile = SPIFFS.open(SEQUENCE_FILE_PATH, "r+");
		deserializeJson(json, sequenceFile);
		json["serial"] = 0;
		currentSequence = -2;
		analyzeRecievedJson();
		sequenceFile.close();
	}
	else
	{
		Serial.println("File not found!");
		animation->addEffect(new FillEffect(animation, {127,0,0}))
			 ->addEffect(new SawtoothEffect(animation, 25, 2000))
			 ->addEffect(new PixEffect(animation, {0, 0, 255}, 500, 1));
	}


	// setup strip
	strip.begin();
	strip.setPixelColor(0,0,255,0);
	strip.show();

	delay(5000);

	mConnectHandler = WiFi.onStationModeConnected([](WiFiEventStationModeConnected e) {
		Serial.println("Wifi connected as event");
	});

	WiFi.onStationModeGotIP([](WiFiEventStationModeGotIP e) {
		Serial.println("Wifi got IP as event");
	});



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
		  newPacket = true;
        });
   } else {
      Serial.println("Failure Listening to UDP");
   }
}

rgb ColorToRGB(uint32_t c) {
  rgb color = {0,0,0};
  color.red = ((c >> 16) & 0xFF);
  color.green = ((c >> 8) & 0xFF);
  color.blue = ((c) & 0xFF);
  return color;
}

void loop()
{
	now = millis();
	if ((now - last_frame) >= frame_delay)
	{
		if (newPacket)
		{
			deserializeJson(json, incomingPacket);
			analyzeRecievedJson();
			newPacket = false;
		}
		last_frame = now;
		animation->render();
		strip.show();
	}
	delay(1);
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

	if (currentSequence > -2)
	{
		sequenceFile = SPIFFS.open(SEQUENCE_FILE_PATH, "w+");
		serializeJson(json, sequenceFile);
		sequenceFile.close();
	}

	if (json["serial"].as<int32_t>() <= currentSequence || json["instance"].as<int16_t>() != currentInstance)
	{
		return;
	}
	currentSequence = json["serial"].as<int32_t>();
	Serial.println("JSON accepted!");
	animation->clearEffects();
	JsonArray effects = json["effects"];
	for (JsonObject effect: effects)
	{
		String effectType = effect["type"].as<String>();
		Serial.println("Found effect: " + effectType);
		IfEffect("fill", FillEffect, ColorToRGB(effect["color"].as<uint32_t>()))
		else IfEffect("sine", SineEffect, effect["w"].as<uint8_t>(), effect["p"].as<uint16_t>())
		else IfEffect("pix", PixEffect, ColorToRGB(effect["color"].as<uint32_t>()),
					  effect["f"].as<uint16_t>(), effect["c"].as<uint8_t>())
	}
}
