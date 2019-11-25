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
	#include <WiFiUdp.h>
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
WiFiUDP Udp;
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
	animation->addEffect(new FillEffect(animation, {127, 0, 0}))
			 ->addEffect(new SineEffect(animation, 25, 2000))
			 ->addEffect(new PixEffect(animation, {0, 0, 255}, 500, 1));

	// setup Wifi
	WiFi.begin("ardulumen", emptyString);

	Udp.setTimeout(1000);

	Udp.begin(3333);

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
}

void loop()
{
	now = millis();
	if ((now - last_frame) >= frame_delay)
	{
		Serial.println("Frame render");
		last_frame = now;
		animation->render();
		strip.show();
	}


	if (Udp.parsePacket())
	{
		Serial.println("Received data");
		int len = Udp.read(incomingPacket, 1024);
		if (len > 0)
		{
			incomingPacket[len] = 0;
		}
		deserializeJson(json, incomingPacket);
		Serial.println((char*)incomingPacket);
		analyzeRecievedJson();
	}
}
/**
 * {
 *	"instance": 0,
 *	"serial": 0,
 *	"effects":
 *		[
 *			{
 *				"type": "fill",
 *				"color": 16711680
 *			},
 *			{
 *				"type": "sine",
 *				"w": 25,
 *				"p": 2000
 *			},
 *			{
 *				"type": "pix",
 *				"color": 16711680,
 *				"f": 200,
 *				"c": 1
 *			}
 *		]
 * }
 */
void analyzeRecievedJson()
{
	if (currentInstance == -1)
	{
		currentInstance = json["instance"].as<int16_t>();
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
		IfEffect("fill", FillEffect, ColorToRGB(effect["color"].as<uint32_t>()))
		else IfEffect("sine", SineEffect, effect["w"].as<uint8_t>(), effect["p"].as<uint16_t>())
		else IfEffect("pix", PixEffect, ColorToRGB(effect["color"].as<uint32_t>()), effect["f"].as<uint16_t>(), effect["c"].as<uint8_t>())
	}
}


/**
 *
   ▒cl`{▒o$phardulumen v0.0.1-dev
WiFi connected!
Duration: 43
Connected to http://192.168.4.1/led
{"instance":0,"serial":0,"effect":1,"filename":"/effect1.json","effects":[{"type":"fill","color":512},{"type":"sine","w":25,"p":2000},{"type":"pix","color":16711680,"f":200,"c":1}]}
JSON accepted!
*/
