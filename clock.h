/*  
 *  Clock library
 *   
 *   This library takes care of the core handling of the clock
 *   - The binding of the LED's within the clock and the driving of the Led's in the led.h library
 *   - Handling Hours, Minutes and Seconds to act together as they represent the clock
 *   - Handling the timings and the triggering of the speech which is driven by the speech.h library
 * 
 *  Functions:
 *    updateLedBackground()     -- Setting the Quarters in a led color to get a better visiualisation of the clock
 *    determineLedPositions()   -- Determining the Hour / Minute / Second led position and checking if they overlap
 *    
 *    updateHourLed()           -- Setting the Hour led
 *    updateMinuteLed()         -- Setting the Minute led
 *    updateSecondLed()         -- Setting the Second led
 *    
 *    displayCurrentTime()      -- Orchestrating the calling of all determinations and setting the lighting of the leds
 *                                 Also handling the connection to the RTC and indicating if this connection is broken
 *                                 
 *    update()                  -- The method that handles getting the latest time state and actually showing the leds                             
 *    
 */

#define SECONDSRISE               1  // This is the rise step of your led (Modify depending on the update speed of your Arduino; smaller for slower rise)
#define SECONDSBLINKEACH       1000  // Note this is the speed in milliseconds

#define MINUTESVALUE             92  // The brightness of minutes
#define HOURSVALUE               92  // The brightness of hours
#define QUARTERVALUE             16  // The brightness of the quarters
#define SECONDSMINVALUE           1  // The starting brightness of seconds
#define SECONDSMAXVALUE          32  // The maximum brightness of seconds

/*
#define MINUTESVALUE              4  // The brightness of minutes
#define HOURSVALUE                4  // The brightness of hours
#define QUARTERVALUE              4  // The brightness of the quarters
#define SECONDSMINVALUE           1  // The starting brightness of seconds
#define SECONDSMAXVALUE          16  // The maximum brightness of seconds
*/

class Clock {
private:
  float           risevalue = 0;

  unsigned long   previousMillis = millis();

  uint8_t         hourLed;
  uint8_t         minuteLed;
  uint8_t         secondLed;

  // Handle Regular Exception; hours and minuts overlap
  bool            hours_and_minutes_overlap;
  bool            minutes_and_seconds_overlap;
  bool            seconds_and_hours_overlap;
  bool            seconds_and_minutes_overlap;

public:

/* CLOCK BACKGROUND */
void updateLedBackground() {
    for(uint8_t i=0; i<FastLED.size(); i++) { 
    if (  i     != hourLed    && 
          i     != minuteLed  && 
          i     != secondLed  && 
          i % 3 == 0            ) { // If the quarter led doesn't hit the seconds, minutes or hour; show it
        LedArray.setMemoryLedRGB(i, QUARTERVALUE,QUARTERVALUE,0);        
      } else {
        LedArray.setMemoryLedRGB(i, 0,0,0);
      }
    }
}

/* CLOCK PART */
void determineLedPositions() {
    hourLed   = Current.Hour();    
    minuteLed = map(Current.FiveMinute(),0,60,0,12);
    secondLed = map(Current.FiveSecond(),0,60,0,12);

    // Handle Regular Exception; hours and minuts overlap
    hours_and_minutes_overlap   = ( hourLed   == minuteLed );     
    minutes_and_seconds_overlap = ( minuteLed == secondLed );
    seconds_and_hours_overlap   = ( hourLed   == secondLed );
    seconds_and_minutes_overlap = ( minuteLed == secondLed );

}

void updateHourLed() { 

    if ( hours_and_minutes_overlap ) {
      // Manage minutes overlapping hours
      if ( Current.unevenSecond ) {
        LedArray.setMemoryLedRGB(hourLed, LedArray.red[hourLed] + int(risevalue), LedArray.green[hourLed], LedArray.blue[hourLed]); 
      }
    } else {
      // Manage hours overlapping seconds
      if ( not seconds_and_hours_overlap ) {
        LedArray.increaseMemoryLedRGB(hourLed, HOURSVALUE, 0, 0);      
      } else if ( (Current.Second() - Current.FiveSecond()) % 2 != 0 ) {
        LedArray.setMemoryLedRGB(hourLed, LedArray.red[hourLed] + int(risevalue), LedArray.green[hourLed], LedArray.blue[hourLed]); 
      }
    }
    
}

void updateMinuteLed() { 
    if ( hours_and_minutes_overlap ) {
      // Manage minutes overlapping hours
      if ( Current.evenSecond ) {
        LedArray.setMemoryLedRGB(minuteLed, LedArray.red[minuteLed], LedArray.green[minuteLed], LedArray.blue[minuteLed] + int(risevalue) ); 
//        LedArray.setMemoryLedRGB(minuteLed, 0, 0, MINUTESVALUE);
      }
    } else {
      // Manage minutes overlapping seconds
      if ( not seconds_and_minutes_overlap ) {
        LedArray.increaseMemoryLedRGB(minuteLed, 0, 0, MINUTESVALUE);
      } else if ( (Current.Second() - Current.FiveSecond()) % 2 != 0 ) {
        LedArray.setMemoryLedRGB(minuteLed, LedArray.red[minuteLed], LedArray.green[minuteLed], LedArray.blue[minuteLed] + int(risevalue) ); 
//        LedArray.setMemoryLedRGB(minuteLed, 0, 0, MINUTESVALUE);        
      }
    }
}

void updateSecondLed() { 
  
 if ( Current.TimeChanged() ) {
    // Reset the seconds led value
    LedArray.green[secondLed] = SECONDSMINVALUE;
    risevalue = 0;
 }

 // Manage the dimming of the second led in it's domain
 unsigned long modDiff     = (millis() - Current.lastTimeChange ) % SECONDSBLINKEACH;

 if ( modDiff > (SECONDSBLINKEACH * .75) ) {
     risevalue = constrain((risevalue - SECONDSRISE), SECONDSMINVALUE, SECONDSMAXVALUE); 
 } else {  
     risevalue = constrain((risevalue + SECONDSRISE), SECONDSMINVALUE, SECONDSMAXVALUE); 
 }

 // Always show the seconds UNLESS the Seconds, Hours and Minutes ALL overlap
 if ( ( hours_and_minutes_overlap && minutes_and_seconds_overlap ) ) {
      // Show nothing
 } else {
  // Check if seconds and hours overlap or if seconds and minutes overlap
  if ( seconds_and_hours_overlap || seconds_and_minutes_overlap ) {
    if ( (Current.Second() - Current.FiveSecond()) % 2 == 0 ) {
      LedArray.setMemoryLedRGB(secondLed,LedArray.red[secondLed], LedArray.green[secondLed] + int(risevalue) , LedArray.blue[secondLed]); 
    }
  } else {
    LedArray.setMemoryLedRGB(secondLed,LedArray.red[secondLed], LedArray.green[secondLed] + int(risevalue) , LedArray.blue[secondLed]);
  }
 }

}

void displayCurrentTime() {
  if ( Current.check_RTC_OK() ) {
    determineLedPositions();
  
    updateLedBackground();
    updateHourLed();
    updateMinuteLed();
    updateSecondLed();
    
    LedArray.activateMemory();
  } else {
    Serial.println(F("Resetting RTC in 10 seconds..."));

    // Show Red - White for 10 secs before resetting to indicate issues
    for ( uint8_t i = 0 ; i < 20 ; i++ ) {
     if ( i % 2 == 0 ) {
       FastLED.showColor(CRGB(160, 255, 255));
     } else {   
       FastLED.showColor(CRGB(255, 0, 0));
     }
     delay(500);
   }
    Current.reset_RTC();
  }
}

void update() {
    Current.getTime();
    displayCurrentTime();
    FastLED.show();
}

};

Clock LedClock;

