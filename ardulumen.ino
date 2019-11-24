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
HTTPClient client;
StaticJsonDocument<1024> json;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, 2, NEO_GRB + NEO_KHZ800);
PixelPP* animation = new PixelPP(strip.numPixels(), strip.getPixels(), LEDColor::GRB);

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

	//client.setTimeout(50000);

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
		last_frame = now;
		animation->render();
		strip.show();
	}
 	if ((now - last_poll) >= polling_delay)
	{
		last_poll = now;
     // only poll if we have wifi
    if (WiFi.isConnected())
    {
  		uint32_t now2 = millis();
  		String url = "http://" + WiFi.gatewayIP().toString() + ((currentInstance == -1) ? "/led" : ("/led?instance=" + currentInstance));
  		if (client.begin(url))
  		{
  			int httpCode = client.GET();
  			Serial.println("Duration: " + String(millis() - now2));
  			Serial.println("Connected to " + url);
  			if (httpCode == HTTP_CODE_OK)
  			{
  				deserializeJson(json, client.getString());
  				Serial.println(client.getString());
  				analyzeRecievedJson();
  			}
  			else
  			{
  				Serial.println("Error " + String(httpCode) + ": " + client.errorToString(httpCode));
  			}
  			client.end();
  			Serial.println("Connection closed.");
  		}
  		else
  		{
  			Serial.println("Error connecting to " + url);
  		}
    }
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
	Serial.println("JSON accepted!");
	animation->clearEffects();
	JsonArray effects = json["effects"];
	for (int8_t i = 0; i < effects.size(); i++)
	{
		JsonObject effect = effects[i];
		String effectType = effect["type"].as<String>();
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
