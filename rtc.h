/*
 * RTC Library
 * 
 * This takes care of several time related matters
 * - Syncing the PC time to the RTC clock
 * - Syncing the Time from the RTC to the internal clock
 * - Running dynamically from the internal clock i.o. the RTC for "power saving"
 * - Managing DST (Western Europe switch programmed) - even when the clock will be turned off for a few years (Writing DST state to Arduino Nano EEPROM)
 * 
 *  
 * Functions
 *    init_RTC()              -- Initialize the RTC
 *    AssumeDST()             -- Determine based on the RTC's date whether we're in DST or not and correcting if EEPROM has a different DST value
 *    DST_Fix()               -- Checking the current date and time and determine if the moment has come to change DST status
 *    DayOfTheWeek()          -- Determine the day of the week (mo/tu/we/th/fr/sa/su) ; necessary for DST determination
 *    
 *    setRTCTime()            -- Set the RTC Time to the PC system time
 *    reset_RTC()             -- Resetting the connection to the RTC; used when this connection is broken or the RTC has crashed
 *    check_RTC_Status()      -- Checking whether the RTC is still running and store this state
 *    check_RTC_OK()          -- Returning the RTC running state
 *    Sync_ITC()              -- Sync the RTC to the ITC (Internal Clock)
 *    
 *    TimeChanged()           -- Check whether the time has changed from five seconds until the hours
 *    elapsed()               -- Determine whether the amount of milliseconds is allready elapsed
 *    SetNewPreviousTime()    -- Storing the last time that has been shown
 *    getTime()               -- Retrieving the time from the internal time clock
 *    
 *    Second()                -- Getting the current Seconds value  (max 60)
 *    FiveSecond()            -- Determine the FiveSecond value     (max 12)
 *    Minute()                -- Getting the current Minute value   (max 60)
 *    FiveMinute()            -- Determine the FiveMinute value     (max 12)
 *    Hour()                  -- Gettting the current Hour value    (max 12)
 *    
 */

// Pin's connected; NOT CONFIGURABLE!
// Arduino -> RTC
// A4      -> SDA (Default for Nano)
// A5      -> SCL (Default for Nano)

#define EEPROM_DST                0  // The DST EEPROM address; storing the last DST state here

#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>

class Time
{
  private:
    RTC_DS1307    RTC;    // The actual RTC module (RealTime Clock)
    RTC_Millis    ITC;    // The internal arduino "counter"
    DateTime      now;
    
    byte          previous_hour, previous_fiveminute, previous_minute, previous_fivesecond, previous_second;
    bool          RTC_Status = false; // True when RTC is running & connected
  
  public:
    unsigned long lastTimeChange;
    
    bool          evenSecond                      = false;
    bool          unevenSecond                    = not(evenSecond);
    
    bool          ExecuteMinuteChangePattern      = false;
    bool          ExecuteFiveMinuteChangePattern  = false;
    bool          ExecuteHourChangePattern        = false;
    bool          ExecuteQuarterChangePattern     = false;

    bool          DST                             = false;
  
  Time()
  {    
    Wire.begin();   // Start serial simulation
    RTC.begin();    // Initialize the module
  }  
  
  void init_RTC() {
      Serial.println(F("Initializing RTC..."));
      
      if (! RTC.isrunning()) {
        RTC.adjust(DateTime(__DATE__, __TIME__));
        RTC_Status = false;
      } else {
        RTC_Status = true;
      }
            
//    Testing DST functions
//      RTC.adjust(DateTime (2017, 3, 26, 1, 59, 30));      // Test switching to summertime
//      RTC.adjust(DateTime (2017, 10, 29, 2, 59, 30));     // Test switching to wintertime
//      RTC.adjust(DateTime (2017, 10, 29, 12, 14, 55));    // Test switching quarter

      Sync_ITC();

      SetNewPreviousTime();
      AssumeDST();
   }
  

  void AssumeDST() {
    uint8_t DSTSwitchDay;
    
    // Function that is run on initialization that will "Assume" the current DST state based on the date
    // Note that this is only valid if both RTC and Arduino keep running together

    if ( ( now.month() >= 4 ) && ( now.month() <= 10 ) ) {
      DST = true;   // Summer time
    } else {
      DST = false;  // Winter time
    }
    
    // Check if we are in March or October
    if ( ( now.month() == 3 ) || ( now.month() == 10 ) ) {
        for ( DSTSwitchDay = 25; DSTSwitchDay < 32; DSTSwitchDay++ ) {
              // Function stops whenever day 0 (sunday) is found, shouldn't take too long
              if ( DayOfTheWeek(now.year(), now.month(), DSTSwitchDay) == 0 ) { break; }
        }

        Serial.print("Switching DST this month on day ");
        Serial.println(DSTSwitchDay);
        
        // Check if we should switch; if so SWITCH
        if ( now.day() > DSTSwitchDay ) { DST = not(DST); }
        // This goes wrong if the clock is restarted on the day of DST after the DST has actually changed (TOO BAD I need some debugging space too!)
    }

    if ( DST == true ) {
      Serial.println(F("We are in summer time now!"));
    } else {
      Serial.println(F("We are in winter time now!"));
    }

    if  ( EEPROM.read(EEPROM_DST) != DST ) {
      if ( DST == true ) {
        Serial.println(F("Stored DST setting was Winter Time; adjusting RTC to match Summer time now!"));
        RTC.adjust(DateTime (now.year(), now.month(), now.day(), now.hour() + 1, now.minute(), now.second()));
      } else {
        Serial.println(F("Stored DST setting was Summer Time; adjusting RTC to match Winter time now!"));
        RTC.adjust(DateTime (now.year(), now.month(), now.day(), now.hour() - 1, now.minute(), now.second()));
      }

      // Correcting the currently stored DST state
      EEPROM.write(EEPROM_DST, DST);
    } else {
      Serial.println(F("EEPROM agrees with the current time; no adjustment needed"));
    }
  }

  void DST_Fix() {
    // Source: http://www.instructables.com/id/The-Arduino-and-Daylight-Saving-Time-Europe/
    if (DayOfTheWeek(now.year(), now.month(), now.day()) == 0 && now.month() == 10 && now.day() >= 25 && now.hour() == 3 && DST==true)  // Switch to winter time in October
    {
      //      setclockto 2 am; // 1 hour back
      Serial.println(F("Adjusting time to match Winter time now"));
      RTC.adjust(DateTime (now.year(), now.month(), now.day(), now.hour() - 1, now.minute(), now.second()));
      DST=false;
      getTime();
      SetNewPreviousTime();
    } 
  
    if (DayOfTheWeek(now.year(), now.month(), now.day()) == 0 && now.month() == 3 && now.day() >= 25 && now.hour() == 2 && DST==false) // Switch to summer time in March
    {
      //      setclockto 3 am; // 1 hour forward
      Serial.println(F("Adjusting time to match Summer time now"));
      RTC.adjust(DateTime (now.year(), now.month(), now.day(), now.hour() + 1, now.minute(), now.second()));
      DST=true;
      getTime();
      SetNewPreviousTime();
    } 

    // Write the current DST status
    Serial.println(F("Writing the DST state to the EEPROM"));
    EEPROM.write(EEPROM_DST, DST);
  }
  
  // Returns day of week for a given date Sunday=0, Saturday=6
  int DayOfTheWeek(int y, int m, int d)
  {     
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    
    y -= m < 3; 
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7; 
  } 

  void setRTCTime() {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

   void Sync_ITC() {
    check_RTC_Status();
    if ( check_RTC_OK() ) {
      Serial.println(F("RTC is ok; syncing..."));
      ITC.begin(RTC.now());
    }
    now = ITC.now();
   }

   void reset_RTC() {
     Serial.println(F("Resetting wire connection"));
     Wire.begin();
     Serial.println(F("Resetting RTC"));
     RTC.begin();
     Serial.println(F("Setting ITC time"));
     Sync_ITC();
     
     Serial.println(F("Determining DST"));
     AssumeDST();

     SetNewPreviousTime();
   }

   void check_RTC_Status() {
      if ( ! RTC.isrunning() ) {
        Serial.println(F("RTC Is not running anymore!"));
        RTC_Status = false;
      } else {
        RTC_Status = true;
      }
   }
  
   bool check_RTC_OK() {
      return RTC_Status;
   }

   bool TimeChanged() {

      // Hour changed?
      if ( Hour() != previous_hour ) {
        ExecuteHourChangePattern        = true;
        DST_Fix();                                // Execute this each hour
      }

      // Five minute changed?
      if ( FiveMinute() != previous_fiveminute )
      {
          switch ( FiveMinute() ) {
            case 15:
            case 30:
            case 45:
            //case 0: // Don't execute when it's a full hour
              Sync_ITC();                     // Sync time from RTC
              ExecuteQuarterChangePattern     = true; 
            break;
            default:
              ExecuteFiveMinuteChangePattern  = true; 
              ExecuteMinuteChangePattern      = true; // Implicit
            break;
          }
      } else if ( Minute() != previous_minute ) {
              ExecuteMinuteChangePattern      = true;
      }
    
      if ( Hour() == previous_hour && FiveMinute() == previous_fiveminute && FiveSecond() == previous_fivesecond ) {
        return false;
      } else {
        SetNewPreviousTime();
        return true;
      }
   }

   void SetNewPreviousTime() {
      previous_hour       = Hour();
      previous_fiveminute = FiveMinute();
      previous_minute     = Minute();
      previous_fivesecond = FiveSecond();
      previous_second     = Second();

      lastTimeChange      = millis();
   }

   void getTime() {
      if ( check_RTC_OK() ) {
        now = ITC.now();
      }
   }

   uint8_t Second() {
      evenSecond    = ( now.second() % 2 == 0 );
      unevenSecond  = not(evenSecond);
      return now.second();
   }

   uint8_t FiveSecond() {
      uint8_t seconds = Second() / 5;
      return  seconds * 5;
   }

   uint8_t Minute() {
      //return 30;
      return now.minute();
   }

   uint8_t FiveMinute() {
      uint8_t fivemin = Minute() / 5;
      return fivemin * 5;
   }

   uint8_t Hour() {
      uint8_t minus = 0;
      
      if ( now.hour() >= 12 ) { minus = 12; }
      //return 6;
      return now.hour() - minus;
   }

  bool elapsed(unsigned long starttime, unsigned long elapse_time)
  {
    if ( (millis() - starttime) > elapse_time )
    { return true;  } 
    else
    { return false; }
  }
   
};

Time Current;



