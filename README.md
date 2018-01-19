[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/FastLED/public)
-------------------------------------------------
18/01/2018 UPDATE: standard FastLED now includes esp32 support as of version 3.1.8! Get it at https://github.com/FastLED/FastLED, happy lighting!
-------------------------------------------------
This repo now contains experimental support for parallel data output on ESP32; currently (01/18/18) hardcoded out put for pins 12-19,0,2,3,4,5,21,22,23  based from https://github.com/eshkrab/FastLED-esp32

The strip 1 will be wired to pin 12
The strip 2 will be wired to pin 13
....
The strip 16 will be wired to pin 23

For esp32 I Suggest you use the code done by Sam Guyer https://github.com/samguyer  it avoids flickering




// -- The core to run FastLED.show()
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
FASTLED_USING_NAMESPACE
#define FASTLED_SHOW_CORE 0



#define NUM_STRIPS 11  //from 1 to 16
#define NUM_LEDS_PER_STRIP  200

#define NUM_LEDS NUM_STRIPS * NUM_LEDS_PER_STRIP
CRGB leds[NUM_LEDS];


// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;


void FastLEDshowESP32()
{
if (userTaskHandle == 0) {
const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
// -- Store the handle of the current task, so that the show task can
//    notify it when it's done
userTaskHandle = xTaskGetCurrentTaskHandle();

// -- Trigger the show task
xTaskNotifyGive(FastLEDshowTaskHandle);

// -- Wait to be notified that it's done
ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
userTaskHandle = 0;
}
}

void FastLEDshowTask(void *pvParameters)
{
const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 500 );
// -- Run forever...
for(;;) {
// -- Wait for the trigger
ulTaskNotifyTake(pdTRUE,portMAX_DELAY);

// -- Do the show (synchronously)
FastLED.show();

// -- Notify the calling task
xTaskNotifyGive(userTaskHandle);
}

}
Then add code to initialize the task in your setup() function:

void setup(){
// -- Create the FastLED show task
xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

//-- Initiate the Leds.
FastLED.addLeds<WS2811_PORTA,NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);



}


Finally, call the new show function in place of regular show:


void loop(){

static uint8_t hue = 0;
for(int i = 0; i < NUM_STRIPS; i++) {
for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
leds[(i*NUM_LEDS_PER_STRIP) + j] = CHSV((32*i) + hue+j,192,255);
}
}

// Set the first n leds on each strip to show which strip it is
for(int i = 0; i < NUM_STRIPS; i++) {
for(int j = 0; j <= i; j++) {
leds[(i*NUM_LEDS_PER_STRIP) + j] = CRGB::Red;
}
}

hue++;

// send the 'leds' array out to the actual LED strip
FastLEDshowESP32();
// FastLED.show();
}



Here are the performances:
![alt text](https://github.com/hpwit/fastled-esp32-16PINS/blob/master/Perf.png?raw=true)




-------------------------------------------------
IMPORTANT NOTE: For AVR based systems, avr-gcc 4.8.x is supported and tested.  This means Arduino 1.6.5 and later.


FastLED 3.1
===========

This is a library for easily & efficiently controlling a wide variety of LED chipsets, like the ones
sold by adafruit (Neopixel, DotStar, LPD8806), Sparkfun (WS2801), and aliexpress.  In addition to writing to the
leds, this library also includes a number of functions for high-performing 8bit math for manipulating
your RGB values, as well as low level classes for abstracting out access to pins and SPI hardware, while
still keeping things as fast as possible.  Tested with Arduino up to 1.6.5 from arduino.cc.

Quick note for people installing from GitHub repo zips, rename the folder FastLED before copying it to your Arduino/libraries folder.  Github likes putting -branchname into the name of the folder, which unfortunately, makes Arduino cranky!

We have multiple goals with this library:

* Quick start for new developers - hook up your leds and go, no need to think about specifics of the led chipsets being used
* Zero pain switching LED chipsets - you get some new leds that the library supports, just change the definition of LEDs you're using, et. voila!  Your code is running with the new leds.
* High performance - with features like zero cost global brightness scaling, high performance 8-bit math for RGB manipulation, and some of the fastest bit-bang'd SPI support around, FastLED wants to keep as many CPU cycles available for your led patterns as possible

## Getting help

If you need help with using the library, please consider going to the google+ community first, which is at http://fastled.io/+ - there are hundreds of people in that group and many times you will get a quicker answer to your question there, as you will be likely to run into other people who have had the same issue.  If you run into bugs with the library (compilation failures, the library doing the wrong thing), or if you'd like to request that we support a particular platform or LED chipset, then please open an issue at http://fastled.io/issues and we will try to figure out what is going wrong.

## Simple example

How quickly can you get up and running with the library?  Here's a simple blink program:

	#include "FastLED.h"
	#define NUM_LEDS 60
	CRGB leds[NUM_LEDS];
	void setup() { FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS); }
	void loop() {
		leds[0] = CRGB::White; FastLED.show(); delay(30);
		leds[0] = CRGB::Black; FastLED.show(); delay(30);
	}

## Supported LED chipsets

Here's a list of all the LED chipsets are supported.  More details on the led chipsets are included *TODO: Link to wiki page*

* Adafruit's DotStars - AKA the APA102
* Adafruit's Neopixel - aka the WS2812B (also WS2811/WS2812/WS2813, also supported in lo-speed mode) - a 3 wire addressable led chipset
* TM1809/4 - 3 wire chipset, cheaply available on aliexpress.com
* TM1803 - 3 wire chipset, sold by radio shack
* UCS1903 - another 3 wire led chipset, cheap
* GW6205 - another 3 wire led chipset
* LPD8806 - SPI based chpiset, very high speed
* WS2801 - SPI based chipset, cheap and widely available
* SM16716 - SPI based chipset
* APA102 - SPI based chipset
* P9813 - aka Cool Neon's Total Control Lighting
* DMX - send rgb data out over DMX using arduino DMX libraries
* SmartMatrix panels - needs the SmartMatrix library - https://github.com/pixelmatix/SmartMatrix


LPD6803, HL1606, and "595"-style shift registers are no longer supported by the library.  The older Version 1 of the library ("FastSPI_LED") has support for these, but is missing many of the advanced features of current versions and is no longer being maintained.


## Supported platforms

Right now the library is supported on a variety of arduino compatable platforms.  If it's ARM or AVR and uses the arduino software (or a modified version of it to build) then it is likely supported.  Note that we have a long list of upcoming platforms to support, so if you don't see what you're looking for here, ask, it may be on the roadmap (or may already be supported).  N.B. at the moment we are only supporting the stock compilers that ship with the arduino software.  Support for upgraded compilers, as well as using AVR studio and skipping the arduino entirely, should be coming in a near future release.

* Arduino & compatibles - straight up arduino devices, uno, duo, leonardo, mega, nano, etc...
* Arduino Yún
* Adafruit Trinket & Gemma - Trinket Pro may be supported, but haven't tested to confirm yet
* Teensy 2, Teensy++ 2, Teensy 3.0, Teensy 3.1/3.2, Teensy LC - arduino compataible from pjrc.com with some extra goodies (note the teensy 3, 3.1, and LC are ARM, not AVR!)
* Arduino Due and the digistump DigiX
* RFDuino
* SparkCore
* Arduino Zero
* ESP8266 using the arduino board definitions from http://arduino.esp8266.com/stable/package_esp8266com_index.json - please be sure to also read https://github.com/FastLED/FastLED/wiki/ESP8266-notes for information specific to the 8266.
* The wino board - http://wino-board.com

What types of platforms are we thinking about supporting in the future?  Here's a short list:  ChipKit32, Maple, Beagleboard

## What about that name?

Wait, what happend to FastSPI_LED and FastSPI_LED2?  The library was initially named FastSPI_LED because it was focused on very fast and efficient SPI access.  However, since then, the library has expanded to support a number of LED chipsets that don't use SPI, as well as a number of math and utility functions for LED processing across the board.  We decided that the name FastLED more accurately represents the totality of what the library provides, everything fast, for LEDs.

## For more information

Check out the official site http://fastled.io for links to documentation, issues, and news


*TODO* - get candy
