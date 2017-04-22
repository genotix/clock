/*
 * LED Library  (Uses the FastLED library)
 * 
 * The neopixels addressed through this library have two ways of addressing
 * 1. Through the memory which enables adding colours to each other on the same LED
 * 2. Immediately setting the color to the LED
 * 
 *  Functions: 
 *    init()                  -- Initialize the Neopixels
 *    setMemoryLedRGB()       -- Setting the Memory address of a specific LED with the color value
 *    getMemoryLedRGB()       -- Getting the content of a Memory address countaining the color value of a specific LED
 *    increaseMemoryLedRGB()  -- Adding Color values to another
 *    
 *    activateMemory()        -- Applying the memory color values to the LED's
 *    
 *    setLedRGB()             -- Immediately setting the color value of a specific LED
 *    setAllOff()             -- Immediately disabling all LED's
 *    
 */
 
// Pin's connected
// Arduino       ->  Neopixels LED's
// 2             ->  DATA-IN

#include "FastLED.h"

#define LEDBRIGHTNESS           128

// The amount of leds used
#define NUM_LEDS 12

// Data pin that led data will be written out over
#define DATA_PIN 2
// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

class Led {
public:
  uint8_t red[NUM_LEDS];
  uint8_t green[NUM_LEDS];
  uint8_t blue[NUM_LEDS];
  
  CRGB led_color[NUM_LEDS];

void init() {  
  Serial.println(F("Initializing LED's..."));  
  
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  // Uncomment one of the following lines for your leds arrangement.
  // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(led_color, NUM_LEDS);
  // FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  // FastLED.setBrightness(CRGB(255,255,255));
  // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);

  // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, RGB>(leds, NUM_LEDS);
  //FastLED.addLeds<LPD8806, 9, 10, RGB>(leds, NUM_LEDS);

  // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(LEDBRIGHTNESS);
}

/* LED & Memory management */
// Set the led color in the memory
void setMemoryLedRGB(uint8_t l, uint8_t r, uint8_t g, uint8_t b) {
  red[l]    = r;
  green[l]  = g;
  blue[l]   = b;
}

// Get the color value set in memory for the led
uint8_t getMemoryLedRGB(uint8_t l, char color ) {
  switch ( color ) {
    case 'r': 
      return red[l];
    break;
    case 'g': 
      return green[l];
    break;
    case 'b': 
      return blue[l];
    break;
  }
}

// Add a value to the existing value in the memory block
void increaseMemoryLedRGB(uint8_t l, uint8_t r, uint8_t g, uint8_t b) {
  red[l]    = red[l]   + r;
  green[l]  = green[l] + g;
  blue[l]   = blue[l]  + b;
}

// Immediately set the value of a let
void setLedRGB(uint8_t l, uint8_t r, uint8_t g, uint8_t b) {
  led_color[l] = CRGB(r, g, b);
}

void setAllOff() {
  FastLED.showColor(CRGB(0, 0, 0));
}

/* ACTIVATING THE MEMORY TO THE LEDS */
void activateMemory() {
    for(uint16_t i=0; i<FastLED.size(); i++) {
          setLedRGB(i, red[i], green[i], blue[i]);
    }
}

};

Led LedArray;



