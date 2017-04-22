/*
 * Pattern Library  (Uses the LED.h library)
 * 
 * Mainly used to show different patterns for some nice / funny effects
 * 
 *  Functions:
 *    Intro()           -- The LEDS will show all yellow lighted and will move to 4 Quarters showing only 
 *    QuarterChange()   -- The LEDS will indicate 4 turning Quarters meaning a quarter has passed
 *    RandomLedColors() -- All of the LED's will show random colors - at random
 *    RGBShow()         -- The LEDS always start showing blue when activated; this pattern will show Green and Blue too
 *    
 *    Vu()              -- A pattern that could be a start for something like a VU meter
 *    
 */
  
  
  
#define QUARTERPATTERNTIMEOUT  5000  // How long should the quarter pattern show
#define HOURPATTERNTIMEOUT    20000  // How long should the hour pattern show

void Intro() {
  uint16_t ledintro[] = {
                            0B0000111111111111,
                            0B0000101101101101,
                            0B0000001001001001
                          };

  LedArray.setAllOff();
  
  for( uint8_t l=0; l < 3 ; l++ ) {
    int counter = 0;
    
    for(uint16_t mask=1; counter <= 11 ; mask <<=1) {
            if ( mask & ledintro[l] ) {
              //Serial.println("Showing yellow for led: " + String(counter));
              LedArray.setLedRGB(counter, QUARTERVALUE, QUARTERVALUE, 0);
            } else {
              //Serial.println("Showing black for led: " + String(counter));
              LedArray.setLedRGB(counter, 0, 0, 0);            
            }
            counter++;
    }
    FastLED.show();
    delay(3000);
  }  
}

void QuarterChange(unsigned long TimeOut) {
  uint16_t ledintro[] = {
                            0B0000111000111000,
                            0B0000110001110001,
                            0B0000100011100011,
                            0B0000000111000111,
                            0B0000001110001110,
                            0B0000011100011100
                          };
                          

  LedArray.setAllOff();

  unsigned long startTime = millis();
  
  while ( millis() - startTime < TimeOut ) {
    
    for( uint8_t l=0; l < 6 ; l++ ) {
      int counter = 0;
      
      for(uint16_t mask=1; counter <= 11 ; mask <<=1) {
              if ( mask & ledintro[l] ) {
                //Serial.println("Showing yellow for led: " + String(counter));
                LedArray.setLedRGB(counter, 0, 0, MINUTESVALUE);
              } else {
                //Serial.println("Showing black for led: " + String(counter));
                LedArray.setLedRGB(counter, HOURSVALUE, 0, 0);            
              }
              counter++;
      }
      FastLED.show();
      delay(100);
    }  
  }  
}


void RandomLedColors(unsigned long TimeOut) {
  //LedArray.setAllOff();
  unsigned long startTime = millis();
  
  while ( millis() - startTime < TimeOut ) {
      uint8_t i=random(0,12),r=random(0,128),g=random(0,128),b=random(0,128);

      //Serial.println("LED: "+ String(i) + " - r:" + String(r) + " - g:" + String(g) + " - b:" + String(b));
      
      LedArray.setLedRGB(i,r,g,b);
      FastLED.show();
      delay(100);
  }
  
}


void RGBShow() {
  // Blue is the default starting color; ingoring that...
  FastLED.showColor(CRGB(128, 0, 0));
  delay(2000);
  FastLED.showColor(CRGB(0, 128, 0));
  delay(2000);
}

void Smiley() {
  uint8_t showleds[] = { 1,4,5,6,7,8,11 };

  for(uint8_t i=0; i < sizeof(showleds); i++) {
     LedArray.setLedRGB(showleds[i], 128, 128, 0);
  }

  FastLED.show();
  delay(5000);
}


void Vu() {
  #define STEP1 = { 6 }
  #define STEP2 = { 5,6,7 };
  #define STEP3 = { 4,5,6,7,8 };
  #define STEP4 = { 3,4,5,6,7,8,9 };
  #define STEP5 = { 2,3,4,5,6,7,8,9,10 };
  #define STEP6 = { 1,2,3,4,5,6,7,8,9,10,11 };
  #define STEP7 = { 1,2,3,4,5,6,7,8,9,10,11,12 };

/*
  for(uint8_t i=0; i < sizeof(showleds[]); i++) {
     setLedRGB(showleds[i], 128, 128, 0);
  }
*/
  uint8_t plusone = 6, minusone = 6;  

  for (uint8_t i=0; i < 7 ; i++ ) {
    
    switch ( i ) {
      case 0:
        LedArray.setLedRGB(plusone, 64, 96, 96);       //  6 
        break;
      case 1:
        LedArray.setLedRGB(++plusone, 0, 64, 64);    //  7
        LedArray.setLedRGB(--minusone, 0, 64, 64);   //  5
        break;
      case 2:
        LedArray.setLedRGB(++plusone, 0, 0, 64);    //  8
        LedArray.setLedRGB(--minusone, 0, 0, 64);   //  4
        break;
      case 3:
        LedArray.setLedRGB(++plusone, 32, 64, 32);   //  9
        LedArray.setLedRGB(--minusone, 32, 64, 32);  //  3
        break;
      case 4:
        LedArray.setLedRGB(++plusone, 64, 64, 0);    //  10
        LedArray.setLedRGB(--minusone, 64, 64, 0);   //  2
        break;
      case 5:
        LedArray.setLedRGB(++plusone, 64, 0, 64);   //  11
        LedArray.setLedRGB(--minusone, 64, 0, 64);  //  1
        break;
      case 6:
        LedArray.setLedRGB(--minusone, 64, 16, 16);   //  0
        break;        
      default:
        // nothing
      break;  
    }
    
    delay(1000);
    FastLED.show();
  }
  
}

