/*
 * A simple Clock
 * 
 * Based on: 
 * - 12 x PL9823 8 mm fullcolour LED
 * - Arduino Nano v3 clone
 * - DS1307 Real Time Clock
 * - Serial MP3 Player Catalex (YX5300 chip)
 *  
 * Features:
 * - Indicating Hours, Minutes and Seconds
 * - Provide several animation patterns triggered on time events
 * 
 * - Requires RTC Module
 * - Provides simple DST change for Western Europe
 * 
 * - Supports MP3 player Module
 * - Provides speech per minute (Best to just enable this per hour or so)
 * 
 * by: Eric Oud Ammerveld
 * Sources of code are provided inline
 * 
 */

#include "./rtc.h"
#include "./led.h"
#include "./clock.h"
#include "./speech.h"
#include "./patterns.h"

void setup() {  

  Serial.begin(9600);

  // Internal led showing seconds too
  pinMode(LED_BUILTIN, OUTPUT);

  Current.init_RTC();
  LedArray.init();
  Mp3Speech.init();

    
  // Initializing ALL the colors would be nice here...
  RGBShow();
  Intro();  
  
}

void loop() {

  // Send something through serial input to set the time to the RTC time programmed
  if (Serial.available()) {
    SerialDataProvided();
  }
  
  //Vu();
  //Smiley();
  //RandomLedColors();

  Mp3Speech.update();
  LedClock.update();
  
  if ( Current.ExecuteHourChangePattern ) {
      //Serial.println(F("Hour has changed!"));
      RandomLedColors(HOURPATTERNTIMEOUT); 
      Current.ExecuteHourChangePattern        = false;
      Current.ExecuteQuarterChangePattern     = false;
      Current.ExecuteFiveMinuteChangePattern  = false;
      Current.ExecuteMinuteChangePattern      = false;
      Mp3Speech.Time(Current.Hour(), Current.Minute());
  }
  
  if ( Current.ExecuteQuarterChangePattern ) {
      Serial.println(F("Quarter has changed!"));
      QuarterChange(QUARTERPATTERNTIMEOUT);
      Current.ExecuteQuarterChangePattern     = false;
      Current.ExecuteFiveMinuteChangePattern  = false;
      Current.ExecuteMinuteChangePattern      = false;

  }

  if ( Current.ExecuteFiveMinuteChangePattern ) {
      Serial.println(F("Five Minutes have changed!"));
      Current.ExecuteFiveMinuteChangePattern  = false;
      Current.ExecuteMinuteChangePattern      = false;
  }

  if ( Current.ExecuteMinuteChangePattern ) {
      // Say the time every minute
      //Serial.println(F("Minutes have changed!"));
//      Mp3Speech.Time(Current.Hour(), Current.Minute());
      Current.ExecuteMinuteChangePattern      = false;
  } 

}

void SerialDataProvided() {
    Serial.println(F("Reset the RTC to the PC Clock"));
    Current.setRTCTime();
    
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW); 
    
    while ( Serial.available() ) {   
      Serial.read();    
    }
}




