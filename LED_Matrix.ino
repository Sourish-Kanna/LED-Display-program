// Header file includes
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <Wire.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW     // For Actual Hardware
//#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW // For Wokwi simulator

/*RTC Pins Slot:
SCL A5
SDA A4*/

#define MAX_DEVICES 16  // Define the number of displays connected
#define CLK_PIN 13      // CLK or SCK8
#define DATA_PIN 11     // DATA or MOSI
#define CS_PIN 10       // CS or SS
#define SPEED_TIME 100  // Speed of the transition
#define PAUSE_TIME 0    // Pause Time

// These are for the clock
#define DS1307_ADDRESS 0x68

// Global variables
uint8_t wday, mday, month, year;
uint8_t hours, minutes, seconds;
uint8_t ampm;

char szTime[10];  // mm:ss AM\0
char szMesg[40 + 1] = "Welcome to Computer Department";

uint8_t clear = 0x00;

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void beginDS1307() {
  // Read the values ​​(date and time) of the DS1307 module

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(clear);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 0x07);

  seconds = bcdToDec(Wire.read());
  minutes = bcdToDec(Wire.read());
  hours = bcdToDec(Wire.read() & 0xff);
  wday = bcdToDec(Wire.read());
  mday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
  ampm = 0;

  if(hours > 12)
  {
    hours -= 12;
    ampm = 1;
  }
  else if(hours==12)
  {
    ampm=1;
  }
  else if(hours==0)
  {
    hours = 12;
  }
}

uint8_t bcdToDec(uint8_t value) {
  return ((value / 16 * 10) + (value % 16));
}

// Code for reading clock time
void getTime(char *psz, bool f = true) {
  sprintf(psz, "%02d%c%02d %c%c", hours, (f ? ':' : ' '), minutes,(ampm==1 ? 'P' : 'A'),'M');
}

// Code for reading clock date
void getDate(char *psz) {
  char szBuf[10];
  sprintf(psz, "%d %s %04d", mday, mon2str(month, szBuf, sizeof(szBuf) - 1), (year + 2000));
}

// Get a label from PROGMEM into a char array
char *mon2str(uint8_t mon, char *psz, uint8_t len) {
  static const __FlashStringHelper *str[] = {
    F("Jan"), F("Feb"), F("Mar"), F("Apr"),
    F("May"), F("Jun"), F("Jul"), F("Aug"),
    F("Sep"), F("Oct"), F("Nov"), F("Dec")
  };

  strncpy_P(psz, (const char PROGMEM *)str[mon - 1], len);
  psz[len] = '\0';

  return (psz);
}

char *dow2str(uint8_t code, char *psz, uint8_t len) {
  static const __FlashStringHelper *str[] = {
    F("Sunday"), F("Monday"), F("Tuesday"),
    F("Wednesday"), F("Thursday"), F("Friday"),
    F("Saturday")
  };

  strncpy_P(psz, (const char PROGMEM *)str[code - 1], len);

  psz[len] = '\0';

  return (psz);
}

void setup(void) {
  Wire.begin();

  P.begin(2);
  P.setInvert(false);
  P.setIntensity(1);
  P.displayClear();

  P.setZone(0, 0, 7);
  P.setZone(1, 8, 15);
  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop(void) 
{
  static uint32_t lastTime = 0;  // Memory (ms)
  static uint8_t display = 1;    // Current display mode
  static bool flasher = false;   // Seconds passing flasher

  beginDS1307();

  P.displayAnimate();

  if (P.getZoneStatus(1)) {
    switch (display) {
      case 1:  // Clock
        P.setTextEffect(1, PA_PRINT, PA_NO_EFFECT);
        //P.setTextEffect(1, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        

        if ((millis() - lastTime) >= 1000) {
          lastTime = millis();
          getTime(szTime, flasher);
          flasher = !flasher;
        }

        if ((seconds == 00) && (seconds <= 30)) {
          display++;
          P.setTextEffect(1, PA_PRINT, PA_WIPE_CURSOR);
        }

        break;
      /*case 2:  // Day of week
        P.setFont(0, nullptr);
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        dow2str(wday, szMesg, MAX_MESG);

        break;*/
      default:  // Calendar
        P.setTextEffect(1, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display = 1;
        getDate(szTime);

        break;
    }

    if (P.getZoneStatus(0)) {
    P.displayReset(0);  // Rest zone zero
  }
    
    P.displayReset(1);  // Rest zone one
  }
}
