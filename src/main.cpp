
/* ***********************************************************
 * RTC_Alarm sketch - v1.0
 *   Uses the ZS-040 module, aka DS3231 RTC module
 *   Uses the SimpleAlarmClock.h library by Ricardo Moreno
 *   Uses the LiquidCrystal.h library
 *   Uses the Button.h library by Alexander Brevig & Ricardo Moreno
 *   Uses the pitches.h library
 *   November 25, 2018
 *
 * Inspired by Elegoo Lesson 19 and Simple Projects'
 *   Arduino real time clock with alarm and temperature monitor
 *   using DS3231 and AT24C32 found on ZS-40 module
 *   https://tinyurl.com/y8br3og2
 *   Impliments an alarm clock on a 16x2 LCD Display with
 *   Three tact switches, DS3231 module, LED and passive buzzer.
 *
 * Description:
 *   This sketch illustrates the SimpleAlarmClock library with
 *   DS3231 Real Time Clock and AT24C32 EEPROM.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  History:
 *  11/25/2018 v1.0 - Initial release
 *
 * ********************************************************* */

/* ***********************************************************
 *                         Libraries                         *
 * ********************************************************* */
/* User_Setup.h for TFT_eSPI
#define ST7789_DRIVER
#define TFT_SDA_READ
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

//#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin (could connect to RST pin)
//#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

//#define TFT_BL   32  // LED back-light (required for M5Stack)

#define TFT_INVERSION_ON

//-------------------------------------------------------------------------------------------
#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

#define TFT_SCLK 18
#define TFT_MOSI 23
#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
#define TFT_DC   2  // Data Command control pin
//#define TFT_BL   32  // LED back-light (required for M5Stack)
#define TFT_MISO 19  // Optional - needed for: a) reading screen data, b) reading touch
#define TFT_CS   12 // or 5 or 4  - Chip select control pin - Optional: could be connected to GND - always selected (if touch is not used)
#define TOUCH_CS 13 //Default 21     // Chip select pin (T_CS) of touch screen


#define TFT_INVERSION_ON


*/

#include <Arduino.h>

/* NexConfig.h (for Nextion)
#define DEBUG_SERIAL_ENABLE
#define dbSerial Serial
#define nexSerial Serial2
*/
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "credentials.h"

#include "LVGlue.h"
#include "GUI.h"
//#include <SPI.h>
//#include <TFT_eSPI.h>       // Hardware-specific library
//#include "Nextion.h"

#include <Esp.h>
#include "SimpleAlarmClock.h"          // https://github.com/rmorenojr/SimpleAlarmClock
#include "Button.h"                    // https://github.com/rmorenojr/Button
#include <EEPROM.h>
#include "LightProfile.h"
#include "Lcd_I2C.h"
#include "LedStripMgr.h"

/* ***********************************************************
 *                    LED Control Constants                  *
 * ********************************************************* */
//  C A U T I O N    ! ! !
// ESP32 Datasheet:
// https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
//   2.4 Strapping Pins
//   There are five strapping pins:
//   MTDI   - 12  (MISO)
//   GPIO0
//   GPIO2
//   MTDO   - 15
//   GPIO5
// If you use one of above (in particular GPIO12) you might see:
// "MD5 of file doas not match data in flash"

/* ***********************************************************
 *                    Serial port Pins                       *
 * ********************************************************* */
// Serial1
//  Unusable
// Serial2  (e.g. for Nextion)
//  RX - 16
//  TX - 17


const int Led_CW_Pin = 27;      // PWM
const int Led_WW_Pin = 16;      // PWM
const int Led_G_Pin  = 17;      // PWM
const int Led_R_Pin  = 25;      // PWM
const int Led_B_Pin  = 26;      // PWM
//4 - OK for analog input
const int MultiButton_Pin = 34;

const int SQW_Pin = 35;        // Interrrupt pin

const unsigned EEPROM_SIZE = 2 * LED_LAST;
const unsigned EEPROM_ADDR_MAX_LED_LEVEL = 0x0;

const unsigned LIGHT_LEVEL_ALLOWED_DIFF = 10;
const unsigned AUTO_SWITCH_OFF_TIMEOUT = 3 * 60 * 60 * 1000;

const int16_t YEAR_MIN = 20;
const int16_t YEAR_MAX = 199;
const int16_t YEAR_OFFSET = 2000;

unsigned lightLevelAtBrightening = 0;
boolean prevShouldMoveOn = false;

float getTemperatureValue();
void printLedStatus(int percent, int dir);
void printAlarmIndicators(byte alarmEnabledStatus, byte enabledDows1, byte enabledDows2);
void LedTaskLoop( void * parameter );

void changeYear(DateTime &NowTime, byte Year);
void changeMonth(DateTime &NowTime, byte Month);
void changeDay(DateTime &NowTime, byte Day);
void changeHour(DateTime &NowTime, byte Hour);
void changeMinute(DateTime &NowTime, byte Minute);



LedStripMgr ledMgr(Led_R_Pin, Led_G_Pin, Led_B_Pin, Led_WW_Pin, Led_CW_Pin);

/* ***********************************************************
 *                      Global Constants                     *
 *                    Hardware Definitions                   *
 * ********************************************************* */
//TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
Lcd_I2C lcd;
//NexText t0 = NexText(0,1,"t0");


    const byte RTC_addr=0x68;                // I2C address of DS3231 RTC
    const byte EEPROM_addr=0x57;             // I2C address of AT24C32N EEPROM
    const bool INTCN = true;                 // allows SQW pin to be monitored
      /* I2C address can be found in the datasheet Figure 1. Device
         Address ZS-040 module has pull-up resistors on these pins
         giving them a default value of 1.
         Shorting an individual pad results in different address:
             pads      Binary    Hex    Dec
         | Default  | b1101111 | 0x57 | 87 |
         | short A0 | b1101110 | 0x56 | 86 |
         | short A1 | b1101101 | 0x55 | 85 |
         | short A2 | b1101011 | 0x53 | 83 |
         | All Short| b1101000 | 0x50 | 80 |
         allowing up to eight combinations                      */
    // instantiate SimpleAlarmClock
    SimpleAlarmClock Clock(RTC_addr, EEPROM_addr, INTCN);

    const int DebouceTime = 30;               // button debouce time in ms

    enum Keys { KEY_RIGHT, KEY_SWITCH_MAX, KEY_SWITCH_CUSTOM, KEY_LEFT, KEY_MODE };

    // The following are expected values on Keyboard Pin for:
    // Right (KEY_RIGHT):            0
    // Down  (KEY_SWITCH_MAX):     400
    // Up:   (KEY_SWITCH_CUSTOM): 1100
    // Left  (KEY_LEFT):          1800
    // Menu  (KEY_MODE):          2700
    // no key pressed:            4095
    // So we create a vector with following limits separating buttons readouts:
    Button MultiButton(MultiButton_Pin, BUTTON_MULTIKEY,true, DebouceTime, {300, 800, 1500, 2200, 3300}); // {200, 700, 1400, 2200, 3300});

    const int Button_Hold_Time = 3000;      // button hold length of time in ms
    const int Alarm_View_Pause = 2000;      // View Alarm Length of time in ms
    const unsigned flashInterval = 1000;         // Alarm flashing interval

    //Alarm types:
    const byte Daily=0;
    const byte Weekday=1;
    const byte Weekend=2;
    const byte Once=3;
    //Clocks
    const byte clock0=0;
    const byte alarm1=1;
    const byte alarm2=2;

/* ***********************************************************
 *                      Global Variables                     *
 * ********************************************************* */

    enum States {
        PowerLoss,
        ShowClock,
        ShowAlarm1,
        ShowAlarm2,
        EditClock,
        EditAlarm1,
        EditAlarm2
    };

    States ClockState = ShowClock;
    States PrevState = EditAlarm2;     // Used for debugging

    byte HourType = 0;                // 0=AM/PM, 1=24hour - used in display alarm - to be deleted
    float CurrentTemperature;         // Maybe move as static variable under displayClock function
    unsigned long RunTime = 0;             // Used to track time between get temperature value
    unsigned long buttonHoldPrevTime = 0;  // Used to track button hold times
    unsigned long AlarmRunTime = 0;
    unsigned long lastClockLedPercentShownTime = 0;
    DateTime PreviousTime;            // Maybe move as static variable under displayClock function
    int PreviousLedLevelPercent = -1;
    AlarmTime PreviousAlarm;          // Maybe move as static variable under displayAlarm function
    byte EditHourType = 0;            // 0=AM, 1=PM, 2=24hour - used for edit only
    byte cpIndex = 0;                 // Cursor Position Index - used for edit mode
    bool bHoldButtonFlag = false;     // used to prevent holdButton also activating clickButton
    bool shouldShowPercent = false;

    // For ISR
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    TaskHandle_t LedTask;
volatile unsigned alarmIntrCounter = 0;

/* ***********************************************************
 *                         Functions                         *
 * ********************************************************* */

void displayClock(bool changeFlag=false) {
  /* ***************************************************** *
   * Display clock - skips update if there are no changes
   *
   * Parameters:
   *   changeFlag - true forces display refresh
   *              - false does nothing
   * ***************************************************** */
    bool temperatureChanged = changeFlag;
    bool dateChanged = changeFlag;
    bool timeChanged = changeFlag;

    DateTime NowTime;            //create DateTime struct from Library
    NowTime = Clock.read();      // get the latest clock values
    // CheckFlag Section:
    // The DS3231 temperature can be read once every 64 seconds.
    // Check the temperature every 65 seconds
    unsigned long mills = millis();
    unsigned long uL = mills - RunTime;
    if ((uL >=65000)) {
        float PreviousTemperature = CurrentTemperature;
        CurrentTemperature = getTemperatureValue();
        RunTime = mills;
        if (CurrentTemperature != PreviousTemperature) {
            temperatureChanged = true;
        }
    }

    byte year = gui_get_year() - YEAR_OFFSET;
    if (year != PreviousTime.Year) {
        changeYear(NowTime, year);
        dateChanged = true;
    }
    byte month = gui_get_month();
    if (month != PreviousTime.Month) {
        changeMonth(NowTime, month);
        dateChanged = true;
    }
    byte day = gui_get_day();
    if (day != PreviousTime.Day) {
        // day must be adjusted also if either year (leap-year) or a month has changed
        changeDay(NowTime, day);
        dateChanged = true;
    }

    byte hour = gui_get_hour();
    if (hour != PreviousTime.Hour) {
        changeHour(NowTime, hour);
        timeChanged = true;
    }

    byte minute = gui_get_minute();
    if (minute != PreviousTime.Minute) {
        changeMinute(NowTime, minute);
        timeChanged = true;
    }
    // Check for Time change
    if ((NowTime.Hour != PreviousTime.Hour) ||
        (NowTime.Minute != PreviousTime.Minute))
    {
        timeChanged = true;
    }

    if ((NowTime.Day != PreviousTime.Day) ||
        (NowTime.Month != PreviousTime.Month) ||
        (NowTime.Year != PreviousTime.Year))
    {
        dateChanged = true;
    }

    PreviousTime = NowTime;

    int percent = 0;
    if (shouldShowPercent) {
        // Check for LedLevel change
        percent = ledMgr.getPercent();
        if (PreviousLedLevelPercent != percent) {
          PreviousLedLevelPercent = percent;
          changeFlag = true;
        }
    } else {
        PreviousLedLevelPercent = -1;
    }

    //Update Display - Only change display if change is detected
    if (changeFlag == true){
        if (shouldShowPercent) {
          printLedStatus(percent, ledMgr.getDir());
        } else {
          printAlarmIndicators(Clock.alarmStatus(),  Clock.readAlarm(alarm1).EnabledDows, Clock.readAlarm(alarm2).EnabledDows);
        }
    }

    if (temperatureChanged) gui_set_temperature(CurrentTemperature);
    if (dateChanged) gui_set_date(YEAR_OFFSET + NowTime.Year, NowTime.Month, NowTime.Day);
    if (timeChanged) gui_set_time(NowTime.Hour, NowTime.Minute);
}

void displayAlarm(byte index=1, bool changeFlag=false) {
    /* ***************************************************** *
     * Display Alarm Clock
     *
     * Parameters:
     *   index - the integer value of the alarm clock to
     *           display - 1 or 2
     * ***************************************************** */
    /*  Reminder of AlarmTime Structure:
      struct AlarmTime {
      uint8_t Second;       // 0-59 = 6 bits 0=for alarm2
      uint8_t Minute;       // 0-59 = 6 bits
      uint8_t Hour;         // 0-23 = 5 bits
      uint8_t AlarmMode;    // 0=Daily, 1=Weekday, 2=Weekend, 3=Once
      uint8_t ClockMode;    // 0-2; 0=AM, 1=PM, 2=24hour
      bool Enabled;         // true/false
      }
     */
    /* LCD alarm display pseudo code:
       Alarm 1      ON
       hh:mm AM Daily
       Alarm 1      OFF
       hh:mm PM Weekday
       Alarm 1      ON
       hh:mm AM Weekend
       Alarm 2      ON
       hh:mm 24 Once                                         */
   /*
    AlarmTime alarm;            //create AlarmTime struct from Library

    if (index == alarm2) {
        alarm = Clock.readAlarm(alarm2);      // get the latest alarm2 values
    } else {
        alarm = Clock.readAlarm(alarm1);      // get the latest alarm1 values
    }

    // Check for Alarm change
    if (alarm.Hour != PreviousAlarm.Hour){ changeFlag = true; }
    if (alarm.Minute != PreviousAlarm.Minute){ changeFlag = true; }
    if (alarm.AlarmMode != PreviousAlarm.AlarmMode) { changeFlag = true; }
    if (alarm.EnabledDows != PreviousAlarm.EnabledDows) { changeFlag = true; }

    //Update Display - Only change display if change is detected
    if (changeFlag == true){
        lcd.clear();
        byte enabledDows1 = 0;
        byte enabledDows2 = 0;

        // First row
        lcd.setCursor(0,0);
        if (index == alarm2) {
            lcd.print("Alarm 2");
            enabledDows2 = alarm.EnabledDows;
        } else {
            lcd.print("Alarm 1");
            enabledDows1 = alarm.EnabledDows;
        }
        lcd.setCursor(13,0);
        if (alarm.Enabled == true) {
          lcd.print("ON");
        } else {
          lcd.print("OFF");
        }

        //Second row
        lcd.setCursor(0,1);
        lcd.print(p2Digits(alarm.Hour));
        lcd.print(":");
        lcd.print(p2Digits(alarm.Minute));
        lcd.print(" ");
        switch (alarm.AlarmMode){
            //0=Daily, 1=Weekday, 2=Weekend, 3=Once
            case 0:
                //Daily
                lcd.setCursor(9,1);
                printAlarmIndicators(0b11,  enabledDows1, enabledDows2);
                break;
            case 1:
                //Weekday
                lcd.print(" Weekday");
                break;
            case 2:
                //Weekend
                lcd.print(" Weekend");
                break;
            case 3:
                //Once
                lcd.print(" Once");
                break;
            default:
                //do nothing
                break;
        }
        PreviousAlarm = alarm;
    }*/
}

void changeHour(DateTime &NowTime, byte Hour) {
    Hour %= 24;
    //Serial.print("24Hour = ");Serial.println(Hour);
    NowTime.Hour = Hour;
    Clock.write(NowTime);
/*        case alarm1:
            //alarm1
            alarm.Hour = byte(Hour);
            Clock.setAlarm(alarm,1);
            break;
        case alarm2:
            //alarm2
            alarm.Hour = Hour;
            Clock.setAlarm(alarm,2);
            break;
        default:
            //Clock
            NowTime.Hour = Hour;
            Clock.write(NowTime);
            break;
    }*/
    //Serial.println("End changeHour");
    //TODO: Error checking. Would return 0 for fail and 1 for OK
}

void changeMinute(DateTime &NowTime, byte Minute) {
    Minute %= 60;
    //Serial.println(Minute);
    NowTime.Minute = Minute;
    Clock.write(NowTime);
    //Serial.println("End changeMinute");

    /*
     *      = 1 Alarm1
     *      = 2 Alarm2
     */

    /*switch (i){
        case alarm1:
            //alarm1
            alarm = Clock.readAlarm(alarm1);
            Minute = alarm.Minute;
            break;
        case alarm2:
            //alarm2
            alarm = Clock.readAlarm(alarm2);
            Minute = alarm.Minute;
            break;
        default:
            //Clock
            NowTime = Clock.read();        // get the latest clock values
            Minute = NowTime.Minute;
            break;
    }
    if (increment == true) {
        Minute += 1;
        Minute %= 60;
    } else {
        Minute -= 1;
        Minute %= 60;
    }
    // Note a byte is from 0-255, no negative number
    // that's why we need an int here
    if (Minute < 0) { Minute = 59; }
    switch (i){
        case clock0:
            //Clock
            NowTime.Minute = byte(Minute);
            Clock.write(NowTime);
            break;
        case alarm1:
            //alarm1
            alarm.Minute = byte(Minute);
            Clock.setAlarm(alarm,1);
            break;
        case alarm2:
            //alarm2
            alarm.Minute = byte(Minute);
            Clock.setAlarm(alarm,2);
            break;
        default:
            //Clock
            NowTime.Minute = byte(Minute);
            Clock.write(NowTime);
            break;
    }*/
    //TODO: Error checking. Would return 0 for fail and 1 for OK
}

void changeEnabledDows(byte i, byte dow) {
    /*  Change Dows  1=Sun, 2=Mon,..., 7=Sat
     *    i = 1 Alarm1
     *      = 2 Alarm2
     */

    if ((i==1)||(i=2)){
        AlarmTime alarm = Clock.readAlarm(i);

        alarm.EnabledDows ^= (1 << dow); // invert only 1 bit representing this dow

        Serial.print("changeEnabledDows: ");
        Serial.println(alarm.EnabledDows,BIN);
        Clock.setAlarm(alarm,i);
    }//TODO: Error checking. Would return 0 for fail and 1 for OK
}

void changeYear(DateTime &NowTime, byte Year) {
    if (Year < YEAR_MIN) { Year = YEAR_MAX; }
    if (Year > YEAR_MAX){ Year = YEAR_MIN; }
    NowTime.Year = Year;
    changeDay(NowTime, NowTime.Day);
}

void changeMonth(DateTime &NowTime, byte Month) {
    if (Month > 12) { Month = 1; }
    if (Month < 1) { Month = 12; }
    NowTime.Month = Month;
    changeDay(NowTime, NowTime.Day);
}

void changeDay(DateTime &NowTime, byte Day) {
    byte Month = NowTime.Month;
    uint16_t Year = NowTime.Year + YEAR_OFFSET;
    byte DaysMax = 31;
    switch (Month){
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            DaysMax = 31;
            break;
        case 2:
            DaysMax = 28;
            if ((Year % 4 == 0) && ((Year % 100 != 0) || ( Year % 400 == 0))){
                //those are the conditions to have a leap year
                DaysMax = 29;
            }
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            DaysMax = 30;
            break;
        default:
            break;
    }
    //Serial.print("DaysMax = ");Serial.println(DaysMax);
    if (Day < 1) { Day = DaysMax; }
    if (Day > DaysMax){Day = 1;}
    //Serial.print("changeDay saved = "); Serial.println(Day);
    NowTime.Day = Day;
    Clock.write(NowTime);
}

void toggleShowAlarm(byte i=1){
    if ((i == 1)||(i == 2)){
        if(i == 2){
            ClockState = ShowAlarm2;
        } else {
            ClockState = ShowAlarm1;
        }
        AlarmTime alarm;
        alarm = Clock.readAlarm(i);
        alarm.Enabled = !alarm.Enabled;
        Clock.setAlarm(alarm, i);
        AlarmRunTime = millis();
        displayAlarm(i, true);
    }
    //otherwise do nothing
}


void clearAlarms(void) {
    //Clear alarm flags
    Clock.clearAlarms();
}

void printColorValue(Color_t val) {
  Serial.print('[');
  for (int i = 0; i < LED_LAST; i++) {
    Serial.print(val[i]);
    if (i < LED_LAST -1) {
      Serial.print(',');
    }
  }
  Serial.print(']');
}

void ButtonClick(Button& b){
    //Clocks
    //const byte clock0=0;
    //const byte alarm1=1;
    //const byte alarm2=2;

    //Debug code to Serial monitor
    Serial.print("Button Click - ");
    switch(b.keyValue()){
        case KEY_MODE:
            Serial.println("KEY_MODE");
            break;
        case KEY_LEFT:
            Serial.println("KEY_LEFT");
            break;
        case KEY_RIGHT:
            Serial.print("KEY_RIGHT: ");
            break;
        case KEY_SWITCH_MAX:
            Serial.println("KEY_SWITCH_MAX");
            break;
        case KEY_SWITCH_CUSTOM:
            Serial.println("KEY_SWITCH_CUSTOM");
            break;
        default:
            Serial.println("UNKNOWN");
            //do nothing
            break;
    }

    if (bHoldButtonFlag == true) {
        // After a hold button is released, a button click is also registered
        if (b.keyValue() == KEY_SWITCH_MAX) {
            ledMgr.finishSettingMaxValue();
            Color_t maxLedValue = ledMgr.getMaxValue();
            for (int i = 0; i < LED_LAST; i++) {
              EEPROM.write(EEPROM_ADDR_MAX_LED_LEVEL + 2 * i,maxLedValue[i]);
              EEPROM.write(EEPROM_ADDR_MAX_LED_LEVEL + 2 * i + 1, maxLedValue[i] >> 8);
            }
            EEPROM.commit();
            Serial.print("Saved max alarm LED light value at: ");
            printColorValue(maxLedValue);
        } else {
            Serial.println("Button Click ignored");
        }
        bHoldButtonFlag = false;
    } else {
        if (b.keyValue() == KEY_SWITCH_MAX) {
          ledMgr.handleSwitch();
        }
    }
}

void ButtonHold(Button& b){
    //Clock States:
    // PowerLoss, ShowClock, ShowAlarm1, ShowAlarm2, Alarm, EditClock, EditAlarm1, EditAlarm2
    // static unsigned long buttonHoldPrevTime = 0;

    //Debug code to Serial monitor
    Serial.print("Button Hold - ");
    switch(b.keyValue()){
        case KEY_MODE:
            Serial.println("KEY_MODE");
            break;
        case KEY_LEFT:
            Serial.println("KEY_LEFT");
            break;
        case KEY_RIGHT:
            Serial.println("KEY_RIGHT");
            break;
        case KEY_SWITCH_MAX:
            Serial.println("KEY_SWITCH_MAX");
            break;
        case KEY_SWITCH_CUSTOM:
            Serial.println("KEY_SWITCH_CUSTOM");
            break;
        default:
            Serial.println("UNKNOWN");
            //do nothing
            break;
    }

    // To ignore back to back button hold?
    if ((millis()-buttonHoldPrevTime) > 2000){
        if (b.keyValue() == KEY_SWITCH_MAX) {
//            ledMgr.beginSettingMaxValue();
//            Serial.println("Setting max alarm LED light value started...");
//            bHoldButtonFlag = true;
        }
    }
}

float getTemperatureValue(){
    // Value from Clock.getTemperatureFloat() in  Celsius
    return Clock.getTemperatureFloat();
}

void IRAM_ATTR AlarmIntrCallback() {
  portENTER_CRITICAL(&mux);
  alarmIntrCounter++;
  Serial.print("I S R !!!! ");
  portEXIT_CRITICAL(&mux);
}

byte CheckAlarmStatus(){
    /* Returns:
     0 - No alarms
     1 - Alarm 1 enabled
     2 - Alarm 2 enabled
     3 - Both alarms enabled
    */
    byte flaggedAlarms = 0;
//    bool AlarmStatus = digitalRead(SQW_Pin);
//
//    //INTSQW is Active-Low Interrupt or Square-Wave Output
//    if (AlarmStatus == LOW){
    if (alarmIntrCounter > 0) {

        portENTER_CRITICAL(&mux);
        alarmIntrCounter--;
        portEXIT_CRITICAL(&mux);

        //Alarm detected
        Serial.println("Alarm detected");
        flaggedAlarms = Clock.flaggedAlarms();

        if (flaggedAlarms) {
            Serial.print("flaggedAlarms: ");
            Serial.println(flaggedAlarms);
            if (flaggedAlarms & 1) {
                AlarmTime alarm = Clock.readAlarm(alarm1);
                Serial.print("Alarm 1 Enabled Dows: ");
                Serial.println(alarm.EnabledDows);
            }
            if (flaggedAlarms & 2) {
                AlarmTime alarm = Clock.readAlarm(alarm2);
                Serial.print("Alarm 2 Enabled Dows: ");
                Serial.println(alarm.EnabledDows);
            }
        }

        //turn off alarms
        Serial.println("Turning alarms off");
        clearAlarms();
    }

    return flaggedAlarms;
}

void printAlarmIndicators(byte alarmEnabledStatus, byte enabledDows1, byte enabledDows2){
    /* Returns:
       0 - No alarms
       1 - Alarm 1 enabled
       2 - Alarm 2 enabled
       3 - Both alarms enabled
     */
    static char dowLetters[] = {'-', 'P', 'W',  'S',  'C',  'P',  'S',  'N'};
    for (byte i = 2; i <=8; i++) {
        byte enabledStatus = alarmEnabledStatus;
        byte dow = (i > 7)? 1 : i;

        byte alarmEnabledOnDow = (enabledDows1 >> dow) & 1;
        if (!alarmEnabledOnDow) {
          enabledStatus &= 0b10;
        }

        alarmEnabledOnDow = (enabledDows2 >> dow) & 1;
        if (!alarmEnabledOnDow) {
          enabledStatus &= 0b01;
        }
        switch (enabledStatus){
            case 0:
                //No alarms
                lcd.print(dowLetters[i-1]);
                break;
            case 1:
                //alarm 1 enabled
                lcd.write(LCD_CHAR_ALARM1); //cA1
                break;
            case 2:
                //alarm 2 enabled
                lcd.write(LCD_CHAR_ALARM2); //cA2
                break;
            case 3:
                //both alarms enabled
                lcd.write(LCD_CHAR_BOTH_ALARMS); //cBA
                break;
            default:
                break;
        }
    }
}

void printLedStatus(int percent, int dir) {
  lcd.print("  ");

  if ((dir == 0)) {
    lcd.print(' ');
  } else if (dir == 1) {
    lcd.write(LCD_CHAR_UP_ARROW);
  } else {
    lcd.write(LCD_CHAR_DOWN_ARROW);
  }

  if (percent < 100) {
    lcd.write(' ');
  }
  if (percent < 10) {
    lcd.write(' ');
  }
  lcd.print(percent);
  lcd.write('%');
}

/* ***********************************************************
 *                         Web Server                        *
 * ********************************************************* */
WebServer server(80);


void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
void serverSetup(void) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

/* ***********************************************************
 *                         Void Setup                        *
 * ********************************************************* */
void setup() {
    // Get the start time
    RunTime = millis();

    /*         Nextion Display          */
    //nexInit(500000,115200); // 500000 is for Serial (debug dbSerial).115200 is for Serial2 (nexSerial).

    //Serial Monitor
    Serial.begin(500000);

    Serial.println("");
    Serial.println("Setup Begin");

    log_d("Total heap: %d", ESP.getHeapSize());
    log_d("Free heap: %d", ESP.getFreeHeap());
    log_d("Total PSRAM: %d", ESP.getPsramSize());
    log_d("Free PSRAM: %d", ESP.getFreePsram());

    log_i("DUTY_MAX: %u", DUTY_MAX);

    EEPROM.begin(EEPROM_SIZE);

    ledMgr.init();

    /*         Pin Modes            */

    pinMode(SQW_Pin, INPUT);

    Color_t maxLedValue;
    for (int i = 0; i < LED_LAST; i++) {
      byte maxLedValueLow  = EEPROM.read(EEPROM_ADDR_MAX_LED_LEVEL + i*2);
      byte maxLedValueHigh = EEPROM.read(EEPROM_ADDR_MAX_LED_LEVEL + i*2 + 1);
      uint16_t component = ((uint16_t)maxLedValueHigh << 8) + (uint16_t)maxLedValueLow;
      maxLedValue[i] = component;
    }
    maxLedValue[0] = 1000;
    maxLedValue[1] = 700;
    maxLedValue[2] = 100;
    maxLedValue[3] = DUTY_MAX;
    maxLedValue[4] = 2000;
/*    maxLedValue[0] = 500;
    maxLedValue[1] = 350;
    maxLedValue[2] = 50;
    maxLedValue[3] = DUTY_MAX;
    maxLedValue[4] = 2000;*/
    ledMgr.setMaxValue(maxLedValue);
    Serial.print("Max Alarm LED value: ");
    printColorValue(maxLedValue);

    /*          LCD Stuff           */
    lcd.init();                      // initialize the lcd


    /*         Clock Stuff          */
    Clock.begin();
    //Clock.setInterruptCtrl(true);

    CurrentTemperature = getTemperatureValue();

    /*  Button callback functions   */
    MultiButton.clickHandler(ButtonClick);
    MultiButton.holdHandler(ButtonHold,Button_Hold_Time);

    xTaskCreatePinnedToCore(
      LedTaskLoop,
      "LedTaskLoop",
      8000,
      NULL,
      1,
      &LedTask,
      0);
    delay(500);  // needed to start-up task1

    attachInterrupt(digitalPinToInterrupt(SQW_Pin), AlarmIntrCallback, FALLING);

    serverSetup();

    setup_lvglue();
    setup_gui();

    if (Clock.getOSFStatus() == true){
        //Restart from power loss detected
        gui_show_datetime_view();
        Clock.clearOSFStatus();
        Serial.println("PowerLoss State");
    }

    //Display the clock
    displayClock(true);


    //Debug code
    Serial.print("Register 0x0E = ");Serial.println(Clock.getCtrlRegister(), BIN);
    Serial.print("Register 0x0F = ");Serial.println(Clock.getStatusRegister(), BIN);
    Serial.println("Setup End");
}

/* ***********************************************************
 *                         Void Loop                         *
 * ********************************************************* */
void loop() {
    //if (ClockState != PrevState) { Serial.print("ClockState = ");Serial.println(ClockState); PrevState = ClockState;}

    switch (ClockState){
        case PowerLoss:
            if (ClockState != PrevState) { Serial.println("ClockState = PowerLoss"); PrevState = ClockState;}
            break;
        case ShowClock:
            if (ClockState != PrevState) { Serial.println("ClockState = ShowClock"); PrevState = ClockState;}
            break;
        default:
            Serial.println("ClockState = default!!");
            break;
    }
    displayClock();

    bool shouldMoveOn = ledMgr.shouldMoveOn();
    if (shouldMoveOn) {
      shouldShowPercent = true;
    } else {
      if (prevShouldMoveOn) {
        shouldShowPercent = false;
        log_d("Max Level reached at %d%%", ledMgr.getPercent());
      }
    }
    prevShouldMoveOn = shouldMoveOn;

    MultiButton.process();

    byte activeAlarms = CheckAlarmStatus();  //Returns which alarms are activated
    if (activeAlarms) {
        Serial.print("Active alarms: ");
        Serial.println(activeAlarms);
        ledMgr.setDirAndProfile(1, LightProfileName::Alarm);
    }

    //t0.setText("Test");
    server.handleClient();

    {
      //unsigned long loop_gui_start_mills = millis();
      loop_gui();
      //log_d("GUI duration: %lu", millis() - loop_gui_start_mills);
    }

    delay(2);
}

void LedTaskLoop( void * parameter ) {
    static unsigned long lastLightChangeTime = 0;
    while (true) {
      unsigned long mills = millis();
      unsigned long timeSinceLastLightChange = mills-lastLightChangeTime;
      bool lightChanged = ledMgr.changeLight(timeSinceLastLightChange);
      if (lightChanged) {
        lastLightChangeTime = mills;
        //log_d("LT duration: %lu", millis() - mills);
      } else {
        if ((timeSinceLastLightChange > AUTO_SWITCH_OFF_TIMEOUT) && (ledMgr.getLevel() != 0)) {
          ledMgr.setDirAndProfile(-1, LightProfileName::Alarm);
          log_d("LED auto-switch off.");
        }
      }

      delay(2);
    }
}
