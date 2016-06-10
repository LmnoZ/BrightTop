#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>

//   ISSUES!!! Color weird and only 1 frame animation when button pressed (will advance after another press)
//  Adapted from Neopixel/RGB Shades code for xymapping compatability
//
//   Use Version 3.0 or later https://github.com/FastLED/FastLED
//   ZIP file https://github.com/FastLED/FastLED/archive/master.zip
//
//
//
//Copyright (c) 2015 macetech LLC
//   This software is provided under the MIT License (see license.txt)
//   Special credit to Mark Kriegsman for XY mapping code




#define HWSERIAL      Serial2
#define BLUEFRUIT_UART_MODE_PIN        12 
#define LED_PIN  4
#define BRIGHTNESS  64

// RGB Shades color order (Green/Red/Blue)
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

// Global maximum brightness value, maximum 255
#define MAXBRIGHTNESS 72
#define STARTBRIGHTNESS 102

// Cycle time (milliseconds between pattern changes)
#define cycleTime 15000

// Hue time (milliseconds between hue increments)
#define hueTime 30



// Include FastLED library and other useful files
#include <FastLED.h>
#include "XYmap.h"
#include "utils.h"
#include "effects.h"

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define FRAMES_PER_SECOND  120
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_UART ble(HWSERIAL, BLUEFRUIT_UART_MODE_PIN);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

const int sampleWindow = 50; // Sample window width for the mic in mS (50 mS = 20Hz)
unsigned int sample;
int maximum = 600;

uint8_t red = 0;// Mode and color variables
uint8_t green = 0;
uint8_t blue = 0;
uint8_t lastR = 0;
uint8_t lastG = 0;
uint8_t lastB = 0;
byte mode = 1;
byte button = 0;
int identifier = 0;
byte fadeDelay = 35;
byte color = 0;
byte rainbowDelay = 20;
byte colorAdvance = 0;
unsigned long previousRainbowMillis = 0;
int column;
int row;

int ledArray[16][13] = // Because the Neopixels are in a chain, to access
{ // their adresses in a grid we use a 2D array. Nifty!
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, 66, -1,  -1,  -1},
  { -1, -1, -1, -1, -1, -1, -1, -1, 65, 67, 95,  -1,  -1},
  { -1, -1, -1, -1, -1, -1, -1, 40, 65, 68, 94,  96,  -1},
  { -1, -1, -1, -1, -1, -1, 39, 41, 63, 69, 93,  97, 111},
  { -1, -1, -1, -1, -1, 22, 38, 42, 62, 70, 92,  98, 110},
  { -1, -1, -1, 10, 21, 23, 37, 43, 61, 71, 91,  99, 109},
  { -1,  2,  9, 11, 20, 24, 36, 44, 60, 72, 90, 100,  -1},
  {  1,  3,  8, 12, 19, 25, 35, 45, 59, 73, 89,  -1,  -1},
  {  0,  4,  7, 13, 18, 26, 34, 46, 58, 74, 88,  -1,  -1},
  { -1,  5,  6, 14, 17, 27, 33, 47, 57, 75, 87, 101,  -1},
  { -1, -1, -1, 15, 16, 28, 32, 48, 56, 76, 86, 102, 108},
  { -1, -1, -1, -1, -1, 29, 31, 49, 55, 77, 85, 103, 107},
  { -1, -1, -1, -1, -1, -1, 30, 50, 54, 78, 84, 104, 106},
  { -1, -1, -1, -1, -1, -1, -1, 51, 53, 79, 83, 105,  -1},
  { -1, -1, -1, -1, -1, -1, -1, -1, 52, 80, 82,  -1,  -1},
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, 81, -1,  -1,  -1}
};

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{  
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LAST_VISIBLE_LED + 1);
    FastLED.setBrightness( scale8(currentBrightness, MAXBRIGHTNESS) );

    
  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Connected Macetech LED Shades"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
}

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Color Picker or Controller mode"));

  ble.verbose(false);  // debug info is a little annoying after this point!

  for (uint8_t i = 0; i < NUM_LEDS ; i++) {
    FastLED.clear();
  }
  FastLED.show();
  FastLED.setBrightness(26);// I'd leave this where it is, 26 is pleeenty bright and saves power

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));

}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;
  /* Got a packet! */
  printHex(packetbuffer, len);

  
  if (packetbuffer[1] == 'C')// If the packet is from the color picker, fill the shades with it
  {
    mode = 1;
    red = packetbuffer[2];
    green = packetbuffer[3];
    blue = packetbuffer[4];
    Serial.println ("RGB #");
    if (red < 0x10) Serial.print("0");
    Serial.println(red);
    if (green < 0x10) Serial.print("0");
    Serial.println(green);
    if (blue < 0x10) Serial.print("0");
    Serial.println(blue);

    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i](CRGB::(0, 0, 0));
      FastLED.show();
      delay(20);
    }
  }
  // Buttons
  else if (packetbuffer[1] == 'B')// If a controller button is pressed, read it and assign the mode
  {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    button = buttnum;
    mode = 2;
  }

// Color

  if (mode == 2)// If a button was pressed
  {
    if (button == 1)// First button is a single color fading rainbow
    {
      threeSine();
      FastLED.show();
      FastLED.delay(1000/FRAMES_PER_SECOND);
      }
    
  else if (button == 2)// Second is a ful spectrum rainbow
    {
      glitter();
      FastLED.show();  
      FastLED.delay(1000/FRAMES_PER_SECOND); 
     }

  else if (button == 3)// 
    {
      plasma();
      FastLED.show();  
      FastLED.delay(1000/FRAMES_PER_SECOND);
    }
      
  else if (button == 4);
  {
    slantBars();
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND);
  }
  }
}










