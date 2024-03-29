
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

enum Keys { KEY_ALARM_1, KEY_SWITCH_DOWN, KEY_SWITCH_UP, KEY_ALARM_0, KEY_OFF };

const unsigned EEPROM_SIZE = 2 * LED_LAST * KEY_OFF;
const unsigned EEPROM_ADDR_MAX_LED_LEVEL = 0x0;

const unsigned LIGHT_LEVEL_ALLOWED_DIFF = 10;
const unsigned AUTO_SWITCH_OFF_TIMEOUT = 3UL * 60UL * 60UL * 1000UL;

const int16_t YEAR_MIN = 20;
const int16_t YEAR_MAX = 199;
const int16_t YEAR_OFFSET = 2000;

unsigned lightLevelAtBrightening = 0;
boolean prevShouldMoveOn = false;

float getTemperatureValue();
void LedTaskLoop( void * parameter );

void changeYear(DateTime &NowTime, byte Year);
void changeMonth(DateTime &NowTime, byte Month);
void changeDay(DateTime &NowTime, byte Day);
void changeHour(DateTime &NowTime, byte Hour);
void changeMinute(DateTime &NowTime, byte Minute);
void printColorValue(Color_t val);


LedStripMgr ledMgr(Led_R_Pin, Led_G_Pin, Led_B_Pin, Led_WW_Pin, Led_CW_Pin);

/* ***********************************************************
 *                      Global Constants                     *
 *                    Hardware Definitions                   *
 * ********************************************************* */
//TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
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


    // The following are expected values on Keyboard Pin for:
    // Right (KEY_ALARM_1):          0
    // Down  (KEY_SWITCH_DOWN):    400
    // Up:   (KEY_SWITCH_UP):     1100
    // Left  (KEY_ALARM_0):       1800
    // Menu  (KEY_OFF):           2700
    // no key pressed:            4095
    // So we create a vector with following limits separating buttons readouts:
    Button MultiButton(MultiButton_Pin, BUTTON_MULTIKEY,true, DebouceTime, {300, 1100, 1700, 2500, 3600}); // {200, 700, 1400, 2200, 3300});

    const int Button_Hold_Time = 3000;      // button hold length of time in ms

    //Clocks
    const byte alarm1=1;
    const byte alarm2=2;

/* ***********************************************************
 *                      Global Variables                     *
 * ********************************************************* */


    float CurrentTemperature;         // Maybe move as static variable under displayClock function
    unsigned long RunTime = 0;             // Used to track time between get temperature value
    unsigned long buttonHoldPrevTime = 0;  // Used to track button hold times
    DateTime PreviousTime;            // Maybe move as static variable under displayClock function
    int PreviousLedLevelPercent = -1;
    int PreviousLedDir = 0;
    AlarmTime PreviousAlarm[alarm2 + 1];          // Maybe move as static variable under displayAlarm function
    Color_t old_ledmgr_color;
    Color_t old_gui_color;
    bool bHoldButtonFlag = false;     // used to prevent holdButton also activating clickButton

    // For ISR
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    TaskHandle_t LedTask;
volatile unsigned alarmIntrCounter = 0;

/* ***********************************************************
 *                         Functions                         *
 * ********************************************************* */


Color_t readColorValue(Keys idx) {
    assert(idx < KEY_OFF);
    Color_t retColor;
    for (int i = 0; i < LED_LAST; i++) {
      byte colorValueLow  = EEPROM.read(EEPROM_ADDR_MAX_LED_LEVEL + (idx*LED_LAST + i) * 2);
      byte colorValueHigh = EEPROM.read(EEPROM_ADDR_MAX_LED_LEVEL + (idx*LED_LAST + i) * 2 + 1);
      retColor[i] = ((uint16_t)colorValueHigh << 8) + (uint16_t)colorValueLow;
      if (retColor[i] > DUTY_MAX) {
        retColor[i] = DUTY_MAX;
      }
    }
    return retColor;
}

void writeColorValue(Keys idx, Color_t color) {
    assert(idx < KEY_OFF);
    for (int i = 0; i < LED_LAST; i++) {
        EEPROM.write(EEPROM_ADDR_MAX_LED_LEVEL + (idx*LED_LAST + i) * 2,     color[i]);
        EEPROM.write(EEPROM_ADDR_MAX_LED_LEVEL + (idx*LED_LAST + i) * 2 + 1, color[i] >> 8);
    }
    EEPROM.commit();
    Serial.print("Stored color[");
    Serial.print(idx);
    Serial.print("] value : ");
    printColorValue(color);
}

void displayTemperature(bool changeFlag=false) {
    // CheckFlag Section:
    // The DS3231 temperature can be read once every 64 seconds.
    // Check the temperature every 65 seconds
    unsigned long mills = millis();
    unsigned long uL = mills - RunTime;
    if ((uL >= 65000UL)) {
        float PreviousTemperature = CurrentTemperature;
        CurrentTemperature = getTemperatureValue();
        RunTime = mills;
        if (CurrentTemperature != PreviousTemperature) {
            changeFlag = true;
        }
    }
    if (changeFlag) {
        gui_set_temperature(CurrentTemperature);
    }
}

void displayArrow(bool changeFlag=false) {
    // Check for LedLevel change
    int dir = ledMgr.getDir();
    if (dir != PreviousLedDir) {
        PreviousLedDir = dir;
        changeFlag = true;
    }

    //Update Display - Only change display if change is detected
    if (changeFlag == true){
        gui_set_led_dir(dir);
    }
}

void displayPercent(bool changeFlag=false) {
    // Check for LedLevel change
    int percent = ledMgr.getPercent();
    if (percent != PreviousLedLevelPercent) {
        PreviousLedLevelPercent = percent;
        changeFlag = true;
    }

    //Update Display - Only change display if change is detected
    if (changeFlag == true){
        gui_set_led_percent(percent);
    }
}

void displayClock(bool changeFlag=false) {
  /* ***************************************************** *
   * Display clock - skips update if there are no changes
   *
   * Parameters:
   *   changeFlag - true forces display refresh
   *              - false does nothing
   * ***************************************************** */
    bool dateChanged = changeFlag;
    bool timeChanged = changeFlag;

    DateTime NowTime;            //create DateTime struct from Library
    NowTime = Clock.read();      // get the latest clock values

    if (changeFlag == false) {
        byte year = gui_get_year() - YEAR_OFFSET;
        if (year != PreviousTime.Year) {
            changeYear(NowTime, year);
            //dateChanged = true;
        }
        byte month = gui_get_month();
        if (month != PreviousTime.Month) {
            changeMonth(NowTime, month);
            //dateChanged = true;
        }
        byte day = gui_get_day();
        if (day != PreviousTime.Day) {
            // day must be adjusted also if either year (leap-year) or a month has changed
            changeDay(NowTime, day);
            //dateChanged = true;
        }

        byte hour = gui_get_hour(TIME_ROLLER_IDX);
        if (hour != PreviousTime.Hour) {
            changeHour(NowTime, hour);
            //timeChanged = true;
        }

        byte minute = gui_get_minute(TIME_ROLLER_IDX);
        if (minute != PreviousTime.Minute) {
            changeMinute(NowTime, minute);
            //timeChanged = true;
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
    }

    PreviousTime = NowTime;

    if (dateChanged) gui_set_date(YEAR_OFFSET + NowTime.Year, NowTime.Month, NowTime.Day, NowTime.Dow);
    if (timeChanged) gui_set_time(TIME_ROLLER_IDX, NowTime.Hour, NowTime.Minute);
}

void displayAlarm(byte index, bool changeFlag=false) {
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

    AlarmTime alarm;            //create AlarmTime struct from Library

    assert ((index == alarm1) || (index == alarm2));
    const RollerIndexes_t roller_idx = (index == alarm1)? ALARM_0_ROLLER_IDX : ALARM_1_ROLLER_IDX;
    if (changeFlag == false) {
        alarm.Second = 0;
        alarm.Minute = gui_get_minute(roller_idx);
        alarm.Hour = gui_get_hour(roller_idx);
        alarm.AlarmMode = 0;      // 0=Daily
        alarm.ClockMode = 2;      // 24h
        alarm.Enabled = gui_get_alarm_enabled(roller_idx);
        // Check for Alarm change
        if (alarm.Hour != PreviousAlarm[index].Hour){ changeFlag = true; }
        if (alarm.Minute != PreviousAlarm[index].Minute){ changeFlag = true; }
        if (alarm.Enabled != PreviousAlarm[index].Enabled) { changeFlag = true; }

        for (int i = 0; i <= 1; i++) {
            bool from_status_view = i;
            alarm.EnabledDows = gui_get_alarm_enabled_dows(roller_idx, from_status_view);
            if (alarm.EnabledDows != PreviousAlarm[index].EnabledDows) {
                changeFlag = true;
                break;
            }
        }

        if (changeFlag) {
            Serial.print("Alarm changed - Enabled: ");
            Serial.print(alarm.Enabled);
            Serial.print(", Mode: ");
            Serial.print(alarm.AlarmMode);
            Serial.print(", EnabledDows: ");
            Serial.println(alarm.EnabledDows,BIN);
            Clock.setAlarm(alarm, index);
        }
    }

    //Update Display - Only change display if change is detected
    if (changeFlag == true){
        alarm = Clock.readAlarm(index);
        gui_set_time(roller_idx, alarm.Hour, alarm.Minute);
        gui_set_alarm_enabled_dows(roller_idx, alarm.Enabled, alarm.EnabledDows);
        PreviousAlarm[index] = alarm;
    }
}

void displayColor(bool changeFlag) {
    Color_t color;
    if (changeFlag == false){
        color = gui_get_color();
        if (color != old_gui_color) {
            Serial.println("Detected color change in GUI: ");
            printColorValue(color);
            Serial.println();

            ledMgr.handleLightOn(LightProfileName::Transition, color);

            old_ledmgr_color = old_gui_color = color;
        }
    }

    color = ((ledMgr.getDir() == 1) || ((ledMgr.getDir() == 0) && (ledMgr.getLevel() != 0)))? ledMgr.getTargetValue() : Color_t {};
    if ((changeFlag == false) && (color != old_ledmgr_color)) {
        Serial.print("Detected color change on LEDs: ");
        printColorValue(color);
        Serial.println();

        changeFlag = true;
    }

    if (changeFlag == true){
        old_ledmgr_color = color;

        Serial.print("Setting color in GUI: ");
        printColorValue(color);
        Serial.println();
        gui_set_color(color);

        old_gui_color = color = gui_get_color();
        Serial.print("Color set in GUI: ");
        printColorValue(color);
        Serial.println();
    }
}

void displayAll(bool changeFlag=false) {
    displayTemperature(changeFlag);
    displayArrow(changeFlag);
    displayPercent(changeFlag);
    displayClock(changeFlag);
    displayAlarm(alarm1, changeFlag);
    displayAlarm(alarm2, changeFlag);
    displayColor(changeFlag);
}

void changeMinute(DateTime &NowTime, byte Minute) {
    Minute %= 60;
    //Serial.println(Minute);
    NowTime.Minute = Minute;
    Clock.write(NowTime);
    //Serial.println("End changeMinute");
    //TODO: Error checking. Would return 0 for fail and 1 for OK
}

void changeHour(DateTime &NowTime, byte Hour) {
    Hour %= 24;
    //Serial.print("24Hour = ");Serial.println(Hour);
    NowTime.Hour = Hour;
    Clock.write(NowTime);
    //Serial.println("End changeHour");
    //TODO: Error checking. Would return 0 for fail and 1 for OK
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

void changeMonth(DateTime &NowTime, byte Month) {
    if (Month > 12) { Month = 1; }
    if (Month < 1) { Month = 12; }
    NowTime.Month = Month;
    changeDay(NowTime, NowTime.Day);
}

void changeYear(DateTime &NowTime, byte Year) {
    if (Year < YEAR_MIN) { Year = YEAR_MAX; }
    if (Year > YEAR_MAX){ Year = YEAR_MIN; }
    NowTime.Year = Year;
    changeDay(NowTime, NowTime.Day);
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
    unsigned int key = b.keyValue();
    switch(key) {
        case KEY_OFF:
            Serial.println("KEY_OFF");
            break;
        case KEY_ALARM_0:
            Serial.println("KEY_ALARM_0");
            break;
        case KEY_ALARM_1:
            Serial.println("KEY_ALARM_1");
            break;
        case KEY_SWITCH_DOWN:
            Serial.println("KEY_SWITCH_DOWN");
            break;
        case KEY_SWITCH_UP:
            Serial.println("KEY_SWITCH_UP");
            break;
        default:
            Serial.println("UNKNOWN");
            //do nothing
            break;
    }

    if (bHoldButtonFlag == true) {
        // After a hold button is released, a button click is also registered
        gui_hide_popup();
        Serial.println("Button Click ignored");
        bHoldButtonFlag = false;
    } else {
        if (key < KEY_OFF) {
            Color_t color = readColorValue((Keys)key);
            LightProfileName profName = (ledMgr.getLevel() == 0)? LightProfileName::Switch : LightProfileName::Transition;
            ledMgr.handleLightOn(profName, color);
        } else if (key == KEY_OFF) {
            ledMgr.handleLightOff(LightProfileName::Switch);
        }
    }
}

void ButtonHold(Button& b){
    Serial.print("Button Hold - ");
    const char* storedTxt = "unknown";
    unsigned int key = b.keyValue();
    switch(key){
        case KEY_OFF:
            Serial.println("KEY_OFF");
            break;
        case KEY_ALARM_0:
            Serial.println("KEY_ALARM_0");
            storedTxt = "M+ Alarm 1";
            break;
        case KEY_ALARM_1:
            Serial.println("KEY_ALARM_1");
            storedTxt = "M+ Alarm 2";
            break;
        case KEY_SWITCH_DOWN:
            Serial.println("KEY_SWITCH_DOWN");
            storedTxt = "M+ Switch 1";
            break;
        case KEY_SWITCH_UP:
            Serial.println("KEY_SWITCH_UP");
            storedTxt = "M+ Switch 2";
            break;
        default:
            Serial.println("UNKNOWN");
            //do nothing
            break;
    }

    // To ignore back to back button hold?
    if ((millis()-buttonHoldPrevTime) > 2000UL){
        if (key < KEY_OFF) {
            Color_t color = gui_get_color();
            writeColorValue((Keys)key, color);
            gui_show_popup(storedTxt);
            bHoldButtonFlag = true;
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

    /*         Clock Stuff          */
    Clock.begin();
    clearAlarms();
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
        Clock.resetClock();
        Clock.resetAlarm(alarm1);
        Clock.resetAlarm(alarm2);
        Serial.println("PowerLoss Detected");
    }

    //Display the clock
    displayAll(true);

    //Debug code
    Serial.print("Register 0x0E = ");Serial.println(Clock.getCtrlRegister(), BIN);
    Serial.print("Register 0x0F = ");Serial.println(Clock.getStatusRegister(), BIN);
    Serial.println("Setup End");
}

/* ***********************************************************
 *                         Void Loop                         *
 * ********************************************************* */
void loop() {
    displayAll();

    bool shouldMoveOn = ledMgr.shouldMoveOn();
    if ((!shouldMoveOn) && (prevShouldMoveOn)){
        log_d("Max Level reached at %d%%", ledMgr.getPercent());
    }
    prevShouldMoveOn = shouldMoveOn;

    MultiButton.process();

    byte activeAlarms = CheckAlarmStatus();  //Returns which alarms are activated
    if (activeAlarms) {
        Serial.print("Active alarms: ");
        Serial.println(activeAlarms);
        Color_t color = readColorValue((activeAlarms & 1)? KEY_ALARM_0 : KEY_ALARM_1);
        ledMgr.handleLightOn(LightProfileName::Alarm, color);
    }

    //t0.setText("Test");
    server.handleClient();

    {
      //unsigned long loop_gui_start_mills = millis();
      loop_gui();
      //log_d("GUI duration: %lu", millis() - loop_gui_start_mills);
    }
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
          ledMgr.handleLightOff(LightProfileName::Alarm);
          log_d("LED auto-switch off.");
        }
      }

      delay(2);
    }
}
