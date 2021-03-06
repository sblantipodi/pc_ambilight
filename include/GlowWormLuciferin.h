/*
  GlowWormLuciferin.h - Glow Worm Luciferin for Firefly Luciferin
  All in one Bias Lighting system for PC
  
  Copyright (C) 2020  Davide Perini

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  * Components:
   - Arduino C++ sketch running on an ESP8266EX D1 Mini from Lolin running @ 160MHz
   - WS2812B 5V LED Strip (95 LED)
   - 3.3V/5V Logic Level Converter
   - 220Ω resistor
   - 1000uf capacitor for 5V power stabilization
   - Raspberry + Home Assistant for Web GUI, automations and MQTT server (HA is optional but an MQTT server is needed)
   - Google Home Mini for Voice Recognition (optional)
  NOTE: 3.3V to 5V logic level converter is not mandatory but it is really recommended, without it, 
  some input on the led strip digital pin could be lost. If you use a 5V microcontroller like Arduino Nano or similar you don't need it.
*/

#include <FastLED.h>
#include "Version.h"
#include "BootstrapManager.h"
#if defined(ESP32)
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#endif

/****************** BOOTSTRAP MANAGER ******************/
BootstrapManager bootstrapManager;
Helpers helper;

/************* MQTT TOPICS (change these topics as you wish)  **************************/
String lightStateTopic = "lights/glowwormluciferin";
String updateStateTopic = "lights/glowwormluciferin/update";
String updateResultStateTopic = "lights/glowwormluciferin/update/result";
String lightSetTopic = "lights/glowwormluciferin/set";
String baseStreamTopic = "lights/glowwormluciferin/set/stream";
String streamTopic = "lights/glowwormluciferin/set/stream";
String unsubscribeTopic = "lights/glowwormluciferin/unsubscribe";
const char* CMND_AMBI_REBOOT = "cmnd/glowwormluciferin/reboot";
String fpsTopic = "lights/glowwormluciferin/fps";
String firmwareConfigTopic = "lights/glowwormluciferin/firmwareconfig";
const char* BASE_TOPIC = "glowwormluciferin";
String topicInUse = "glowwormluciferin";

enum class Effect { solid, GlowWorm, GlowWormWifi, bpm, rainbow, solid_rainbow, mixed_rainbow };
Effect effect;

/****************** Glow Worm Luciferin ******************/
unsigned long off_timer;

// DPsoftware Checksum
uint8_t prefix[] = {'D', 'P', 's'}, hi, lo, chk, loSecondPart, usbBrightness, gpio, baudRate, fireflyEffect, i;
bool led_state = true;
uint lastLedUpdate = 10000;
uint lastStream = 0;
float framerate = 0;
float framerateCounter = 0;
int gpioInUse = 5, baudRateInUse = 3, fireflyEffectInUse;
// Upgrade firmware
boolean firmwareUpgrade = false;
size_t updateSize = 0;

/****************** FastLED Defintions ******************/
#define NUM_LEDS    511 // Max Led support
CRGB leds[NUM_LEDS];
int dynamicLedNum = NUM_LEDS;
const String LED_NUM_FILENAME = "led_number.json";
const String GPIO_FILENAME = "gpio.json";
const String TOPIC_FILENAME = "topic.json";
const String BAUDRATE_FILENAME = "baudrate.json";
const String EFFECT_FILENAME = "effect.json";
const String LED_NUM_PARAM = "lednum";
const String GPIO_PARAM = "gpio";
const String MQTT_PARAM = "mqttopic";
const String BAUDRATE_PARAM = "baudrate";
const String EFFECT_PARAM = "effect";

const int FIRST_CHUNK = 190;
const int SECOND_CHUNK = 380;
#define DATA_PIN    5 // Wemos D1 Mini Lite PIN D5
//#define CLOCK_PIN 5
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB

byte realRed = 0;
byte realGreen = 0;
byte realBlue = 0;

byte red = 255;
byte green = 255;
byte blue = 255;
byte brightness = 255;

/****************** GLOBALS for fade/flash ******************/
bool stateOn = false;
bool startFade = false;
bool onbeforeflash = false;
unsigned long lastLoop = 0;
unsigned transitionTime = 0;
bool inFade = false;
int loopCount = 0;
int stepR, stepG, stepB;
int redVal, grnVal, bluVal;

bool flash = false;
bool startFlash = false;
unsigned int flashLength = 0;
unsigned long flashStartTime = 0;
byte flashRed = red;
byte flashGreen = green;
byte flashBlue = blue;

//RAINBOW
uint16_t thishue = 0; // Starting hue value.
uint16_t deltahue = 10;

//NOISE
uint16_t scale = 30;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint16_t maxChanges = 48;      // Value for blending between palettes.
CRGBPalette16 targetPalette(OceanColors_p);
CRGBPalette16 currentPalette(CRGB::Black);

//BPM
uint16_t gHue = 0;

bool breakLoop = false;
int part = 1;

/****************** FUNCTION DECLARATION (NEEDED BY PLATFORMIO WHILE COMPILING CPP FILES) ******************/
// Bootstrap functions
void callback(char* topic, byte* payload, unsigned int length);
void manageDisconnections();
void manageQueueSubscription();
void manageHardwareButton();
// Project specific functions
void sendStatus();
bool processUpdate(StaticJsonDocument<BUFFER_SIZE> json);
bool processJson(StaticJsonDocument<BUFFER_SIZE> json);
bool processFirmwareConfig(StaticJsonDocument<BUFFER_SIZE> json);
bool processGlowWormLuciferinRebootCmnd(StaticJsonDocument<BUFFER_SIZE> json);
bool processUnSubscribeStream(StaticJsonDocument<BUFFER_SIZE> json);
bool swapMqttTopic(StaticJsonDocument<BUFFER_SIZE> json);
void executeMqttSwap(String customtopic);
void setColor(int inR, int inG, int inB);
void checkConnection();
void fadeall();
void showleds();
int calculateStep(int prevValue, int endValue);
int calculateVal(int step, int val, int i);
void mainTask(void * parameter);
void mainLoop();
CRGB Scroll(int pos);
void sendSerialInfo();
void feedTheDog();
void setGpio(int gpio);
void setBaudRate(int baudRate);
void setNumLed(int numLedFromLuciferin);
int setBaudRateInUse(int baudRate);
void swapTopicUnsubscribe();
void swapTopicReplace(String customtopic);
void swapTopicSubscribe();