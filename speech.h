/*
 * Speech Library  (Uses the SoftwareSerial library)
 * 
 * Manages several features for addressing the MP3 player:
 *  - Determining the words forming the Time
 *  - Instructing the MP3 player which sample to play
 *  - Determining when playing a sample has come to an end
 *  - Handling the power management of the MP3 player
 * 
 * Functions
 *    init()          -- Initialize the MP3 player
 *    update()        -- Since this library is continually processed we need to determine if time has come to play the next sample by checking whther the status is Finished playing
 *    
 *    Time()          -- Translates time to the call of an MP3
 *    WordCount()     -- Determine the wordcount of a sentence
 *    clearSentence() -- Empty the sentence (No more talking)
 *    NextWord()      -- Jump to the next word
 *    
 *    playSample()    -- Play a specific MP3 sample
 *    mp3_status()    -- Process the serial buffer and handle the statusses provided
 *    getMp3Status()  -- Request the status of the MP3 player (Acts weird; not using it)
 *    
 *    wake()          -- Change the powerstate of the MP3 player to active
 *    sleep()         -- Change the powerstate of the MP3 player to inactive
 *    reset()         -- Reset the MP3 player
 *    
 *    sendCommand()   -- Send a serial command to the MP3 player
 *    sanswer()       -- Receive a pending response from the MP3 player
 *    printHex()      -- Helper function for showing which Hex address is called to the MP3 player
 *    sbyte2hex()     -- Helper function to translate bytes to Hex
 *    
 *   Source of some of the code below:  https://github.com/cefaloide/ArduinoSerialMP3Player/blob/master/ArduinoSerialMP3Player/ArduinoSerialMP3Player.ino
 *                                      http://www.dx.com/p/uart-control-serial-mp3-music-player-module-for-arduino-avr-arm-pic-blue-silver-342439#.VfHyobPh5z0
 */

// Pin's connected
// Arduino       ->  MP3 module
// A0 (14)       ->  RX
// A1 (15)       ->  TX

#include <SoftwareSerial.h>

#define FOLDER       2    // The folder to access the MP3' in ; nice to randomize this in the future

#define MP3_RX      A1    // Should connect to TX of the Serial MP3 Player module
#define MP3_TX      A0    // Connect to RX of the module

#define EEN         1
#define TWEE        2
#define DRIE        3
#define VIER        4
#define VIJF        5
#define ZES         6
#define ZEVEN       7
#define ACHT        8
#define NEGEN       9
#define TIEN        10
#define ELF         11
#define TWAALF      12
#define DERTIEN     13
#define VEERTIEN    14 
#define HALF        15
#define KWART       16
#define HET_IS_NU   17
#define OVER        18
#define VOOR        19
#define UUR         20

SoftwareSerial Mp3Serial(MP3_RX, MP3_TX);

String mp3Answer;           // Answer from the MP3.

/************ Command byte **************************/
#define CMD_NEXT_SONG         0X01  // Play next song.
#define CMD_PREV_SONG         0X02  // Play previous song.
#define CMD_PLAY_W_INDEX      0X03  // Play with index
#define CMD_VOLUME_UP         0X04
#define CMD_VOLUME_DOWN       0X05
#define CMD_SET_VOLUME        0X06

#define CMD_SNG_CYCL_PLAY     0X08  // Single Cycle Play.
#define CMD_SEL_DEV           0X09  // Select SD Card to storage device
#define CMD_SLEEP_MODE        0X0A
#define CMD_WAKE_UP           0X0B
#define CMD_RESET             0X0C
#define CMD_PLAY              0X0D
#define CMD_PAUSE             0X0E
#define CMD_PLAY_FOLDER_FILE  0X0F

#define CMD_STOP_PLAY         0X16 // Stop / Play
#define CMD_FOLDER_CYCLE      0X17 // Loop through folders when reaching the last
#define CMD_SHUFFLE_PLAY      0x18 // Set shuffle
#define CMD_SET_SNGL_CYCL     0X19 // Set single cycle.

#define CMD_SET_DAC           0X1A
#define DAC_ON                0X00
#define DAC_OFF               0X01

#define CMD_PLAY_W_VOL        0X22
#define CMD_PLAYING_N         0x4C
#define CMD_QUERY_STATUS      0x42  // Check the status of the MP3 player
#define CMD_QUERY_VOLUME      0x43  // Get the volume
#define CMD_QUERY_FLDR_TRACKS 0x4e  // Count the total number of folders
#define CMD_QUERY_TOT_TRACKS  0x48  // Count the total number of tracks
#define CMD_QUERY_FLDR_COUNT  0x4f

/************ Options **************************/
#define DEV_TF 0X02

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

class Speech 
{
private:
  uint8_t Sentence[15];
  uint8_t Word          = 0;            // Initialize to the first word
  uint8_t Words         = 0;            // Initialize the wordcount

  bool    MemoryCard    = false;
  bool    Playing       = false;
  bool    Error         = false;
  bool    Ok            = true;
  bool    Sleeping      = false;
  uint16_t LastWordTime;
  
  String  PlayingNumber, FileCount, FolderFileCount, FolderCount;
    
public:   

void init() {
  Serial.println(F("Initializing MP3 player..."));  
  
  Mp3Serial.begin(9600);
  delay(500);
  sendCommand(CMD_SEL_DEV, DEV_TF);
  delay(500);
  
  clearSentence();
}

uint8_t WordCount() {
  uint8_t Count = 0;
  
  for ( uint8_t i=0 ; i < sizeof(Sentence) ; i++ ) {
    if ( Sentence[i] != NULL ) {
      Count ++;
    } else {
      break;
    }
  }
  
//  Serial.println("A Sentence has been loaded of " + String(Count) + " words" + " Playing: " + String(Playing) + " Sleeping: " + String(Sleeping));  
  return Count;
}

void wake(){
  // Serial.println(F("Waking up..."));
  Sleeping  = false;
  sendCommand(CMD_WAKE_UP, 0x00);
  
  if ( Mp3Serial.available() ) {
      mp3_status();                // Process status changes
  }
}

void sleep() {
  //Serial.println(F("Entering sleep mode..."));
  Sleeping  = true;
  sendCommand(CMD_SLEEP_MODE, 0x00);
  
  if ( Mp3Serial.available() ) {
      mp3_status();                // Process status changes
  }
}

void getMp3Status() {
  Serial.println(F("Requesting MP3 status..."));
  sendCommand(CMD_QUERY_STATUS, 0x00);

  if ( Mp3Serial.available() ) {
      mp3_status();                // Process status changes
  }
  delay(500);  
}

void reset() {
  sendCommand(CMD_RESET, 0x00);
  
  if ( Mp3Serial.available() ) {
      mp3_status();                // Process status changes
  }
}

void mp3_status() {
    delay(50);
    // Process the buffer
    while (Mp3Serial.available())
    {
      sanswer();   // Fill the answer buffer...
      switch (ansbuf[3]) {
        case 0x3A:
          MemoryCard  = true;
          Serial.println(F("Memory card inserted"));
          break;
    
        case 0x3B:
          MemoryCard  = false;
          Serial.println(F("Memory card removed"));
          break;      
        
        case 0x3D:
          //Serial.println("Finished playing number: " + String(ansbuf[6], DEC));
          Playing     = false;
          break;
          
        case 0x00:
          // There are some serialisation issues here...
          // Appearantly the status response can come in earlier than expected.
          //Serial.println("Code 0 finished playing number: " + String(ansbuf[6], DEC));
          Playing     = false;
          break;

        case 0x02:
          // There are some serialisation issues here...
          // Appearantly the status response can come in earlier than expected.
          //Serial.println("The sound may still be playing..." + String(ansbuf[6], DEC));
          break;

        case 0x39:
          Serial.println(F("Error playing file"));
          break;          
    
        case 0x40:
          Serial.println(F("Error"));
          Error       = false;
          break;
    
        case 0x41:
          // Serial.println(F("Ok"));
          Ok          = true;          
          break;
    
        case 0x42:
          // Not using this; the thing seems to think it's always playing
          Serial.println(F("Playing"));
          break;
    
        case 0x48:
          FileCount = String(ansbuf[6], DEC);
          Serial.print(F("FileCount "));
          Serial.println(FileCount);
          break;
    
        case 0x4C:
          PlayingNumber = String(ansbuf[6], DEC);
          Serial.print(F("Playing the following song: "));
          Serial.println(PlayingNumber);
          break;
    
        case 0x4E:
          FolderFileCount = String(ansbuf[6], DEC);
          Serial.print(F("FolderFileCount "));
          Serial.println(FolderFileCount);
          break;
    
        case 0x4F:
          FolderCount = String(ansbuf[6], DEC);
          Serial.print(F("FolderCount "));
          Serial.println(FolderCount);
          break;
          
        default:
          // Unknown repsonse 39 seems to lead to error playing
          // Unknown response 1 / 2 / 40
          Serial.print(F("MP3 - Unknown response "));
          Serial.println(ansbuf[3]);
          break;
      }
    }
}

void clearSentence() {
  memset(Sentence, NULL, sizeof(Sentence));
  Word      = 0;
  Words     = WordCount();
}

void playSample(uint8_t Number) {
    Playing = true;
    uint16_t PlayNumber = Number + ( FOLDER * 256 );

    sendCommand(CMD_PLAY_FOLDER_FILE, PlayNumber);
    //sendCommand(CMD_PLAY_W_INDEX, Number);
    delay(100);
}

void NextWord() {
    Word ++;   
    if ( Word > Words || Sentence[Word] == NULL ) {
      // Serial.println(F("Sentence is finished!; clearing sentence"));
      clearSentence();
    } else {
      // No; we can jump to the next word
      // Serial.println("Done with this word; jumping to word " + String(Word)  + " Playing: " + String(Playing) + " Sleeping: " + String(Sleeping));
    }
}

void update() {
  if ( Mp3Serial.available() ) {
    mp3_status();                // Process status changes
  }

  if ( Words > 0 ) {           // There is an array set meaning we have some work to do
    //Serial.println(F("Words found; saying them..."));
    //Serial.println("Sleeping: " + String(Sleeping) + " Playing: " + String(Playing));

    if ( Sleeping ) {
        wake();                      // Wake the MP3 player if it is sleeping
        Playing = false;             // Implicit but might have issues
        mp3_status();                // Process status changes
    }
  
    if ( Playing == false ) {
        // Player will enter Playing == false status through mp3_status() function
        if ( Words >= Word ) { // Not playing but there are still some words left...
          if ( Sentence[Word] != NULL ) {
            //Serial.println("Playing word " + String(Word));
            playSample(Sentence[Word]);
            NextWord();
          }
        }
     }
  }

  // Go to sleep when there is nothing to play and 
  if ( Sleeping == false && Playing == false ) {
    sleep();
  }
  
}

// Library translates time to the call of an MP3
void Time(uint8_t hour, uint8_t minute) {

uint8_t afterhour   = hour;
uint8_t beforehour  = hour;
uint8_t CurrentWord = 0;

  Sentence[CurrentWord] = HET_IS_NU;
  CurrentWord ++;
  
  // Make sure 0 is said as 12
  if ( hour == 0 ) {
    afterhour = 12;
  } else {
    afterhour = hour;
  }
  
  // Make sure the before hour is one bigger
  if ( (hour % 12 ) == 0 ) {
    beforehour = 1;
  }  else {
    beforehour = hour + 1;    
  }  
  
  // Structure for saying the appropriate time
  if ( minute % 15 == 0 ) {
    // Handle quarterly
    switch ( int ( minute / 15 ) ) {
      case 0:
        Serial.print(afterhour);
        Serial.println(F(" uur"));
        
         Sentence[CurrentWord] = afterhour;
         CurrentWord ++;
         Sentence[CurrentWord] = UUR;
         CurrentWord ++;
        break;
      case 1:
        Serial.print(F("kwart over "));
        Serial.println(afterhour);
        
         Sentence[CurrentWord] = KWART;
         CurrentWord ++;        
         Sentence[CurrentWord] = OVER;
         CurrentWord ++;        
         Sentence[CurrentWord] = afterhour;
         CurrentWord ++;
        break;
      case 2:
        Serial.print(F("half "));
        Serial.println(beforehour);   
        
         Sentence[CurrentWord] = HALF;
         CurrentWord ++;        
         Sentence[CurrentWord] = beforehour;
         CurrentWord ++;           
        break;
      case 3:
        Serial.print(F("kwart voor "));
        Serial.println(beforehour);   
        
         Sentence[CurrentWord] = KWART;
         CurrentWord ++;        
         Sentence[CurrentWord] = VOOR;
         CurrentWord ++;        
         Sentence[CurrentWord] = beforehour;
         CurrentWord ++;             
        break;
    }
    
  } else {
    switch ( int ( minute / 15 ) ) {
      case 0: // First quarter
        Serial.print(minute);
        Serial.print(F(" over "));
        Serial.println(afterhour);   
        
         Sentence[CurrentWord] = minute;
         CurrentWord ++;        
         Sentence[CurrentWord] = OVER;
         CurrentWord ++;        
         Sentence[CurrentWord] = afterhour;
         CurrentWord ++;           
      break;
      case 1: // Second quarter
        Serial.print(30 - minute);
        Serial.print(F(" voor half "));
        Serial.println(beforehour);  
          
         Sentence[CurrentWord] = 30 - minute;
         CurrentWord ++;        
         Sentence[CurrentWord] = VOOR;
         CurrentWord ++;        
         Sentence[CurrentWord] = HALF;
         CurrentWord ++;        
         Sentence[CurrentWord] = beforehour;
         CurrentWord ++;                    
      break;
      case 2: // Third quarter
        Serial.print(minute - 30);
        Serial.print(F(" over half "));
        Serial.println(beforehour);     
        
         Sentence[CurrentWord] = minute - 30;
         CurrentWord ++;        
         Sentence[CurrentWord] = OVER;
         CurrentWord ++;        
         Sentence[CurrentWord] = HALF;
         CurrentWord ++;        
         Sentence[CurrentWord] = beforehour;
         CurrentWord ++;         
      break;
      case 3: // Fourth quarter
        Serial.print(60 - minute);
        Serial.print(F(" voor "));
        Serial.println(beforehour);  
        
         Sentence[CurrentWord] = 60 - minute;
         CurrentWord ++;        
         Sentence[CurrentWord] = VOOR;
         CurrentWord ++;        
         Sentence[CurrentWord] = beforehour;
         CurrentWord ++;         
      break;
    }
    
  }

  Words         = WordCount();  // Set the wordcount
  Word          = 0;
}

/********************************************************************************/
/*Function: Send command to the MP3                                         */
/*Parameter:-int8_t command                                                     */
/*Parameter:-int16_ dat  parameter for the command                              */
void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e;   //
  Send_buf[1] = 0xff;   //
  Send_buf[2] = 0x06;   // Len
  Send_buf[3] = command;//
  Send_buf[4] = 0x01;   // 0x00 NO, 0x01 feedback
  Send_buf[5] = (int8_t)(dat >> 8);  //datah
  Send_buf[6] = (int8_t)(dat);       //datal
  Send_buf[7] = 0xef;   //
  //Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++)
  {
    Mp3Serial.write(Send_buf[i]) ;
    //Serial.print(sbyte2hex(Send_buf[i]));
  }
  //Serial.println();
}

/********************************************************************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */
String sanswer(void)
{  
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (Mp3Serial.available() && (i < 10))
  {
    uint8_t b = Mp3Serial.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
  {
    return mp3answer;
  }

  return "???: " + mp3answer;
}


void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.print(tmp);
}



/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                 */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */
String sbyte2hex(uint8_t b)
{
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}


};

Speech Mp3Speech;

