
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
#include <Esp.h>
#include "SimpleAlarmClock.h"          // https://github.com/rmorenojr/SimpleAlarmClock
#include <Button.h>                    // https://github.com/rmorenojr/Button
#include <EEPROM.h>
#include "LightProfile.h"
#include "Lcd_I2C.h"
#include "LedStripMgr.h"

/* ***********************************************************
 *                    LED Control Constants                  *
 * ********************************************************* */


const int LightSensor_Pin = 04;
const int Led_R_Pin  = 12;      // PWM
const int Led_G_Pin  = 15;      // PWM
const int Led_B_Pin  = 33;      // PWM
const int Led_WW_Pin = 5;       // PWM
const int Led_CW_Pin = 13;      // PWM
const int Switch_Pin = 17;
const int Mode_Pin = 16;
const int Lt_Pin = 27;
const int Rt_Pin = 14;
const int LED_Pin = 2;         // digital pin for internal LED
const int SQW_Pin = 26;        // Interrrupt pin

const unsigned EEPROM_SIZE = 2;
const unsigned EEPROM_ADDR_MAX_LED_LEVEL = 0x0;

const unsigned LIGHT_LEVEL_ALLOWED_DIFF = 10;

unsigned lightLevelAtBrightening = 0;
boolean prevShouldMoveOn = false;

LedStripMgr ledMgr(Led_R_Pin, Led_G_Pin, Led_B_Pin, Led_WW_Pin, Led_CW_Pin);

/* ***********************************************************
 *                      Global Constants                     *
 *                    Hardware Definitions                   *
 * ********************************************************* */
Lcd_I2C lcd;
                                             
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
    Button ModeKey(Mode_Pin, BUTTON_PULLUP_INTERNAL, true, DebouceTime);
    Button LtKey(Lt_Pin, BUTTON_PULLUP_INTERNAL, true, DebouceTime);
    Button RtKey(Rt_Pin, BUTTON_PULLUP_INTERNAL, true, DebouceTime);
    Button SwitchKey(Switch_Pin, BUTTON_PULLUP_INTERNAL, true, DebouceTime);

    const int Button_Hold_Time = 3000;      // button hold length of time in ms
    const int Alarm_View_Pause = 2000;      // View Alarm Length of time in ms
    const int SkipClickTime = 60;           // Time in ms to ignore button click
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
    bool bDisplayStatus = true;       // used to track the lcd display on status
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
        if (CurrentTemperature != PreviousTemperature) {changeFlag = true;}
    }

    // Check for Time change
    if (NowTime.Hour != PreviousTime.Hour){ changeFlag = true; }
    if (NowTime.Minute != PreviousTime.Minute){ changeFlag = true; }
    if (NowTime.ClockMode != PreviousTime.ClockMode) { changeFlag = true; }
    if (NowTime.Day != PreviousTime.Day) { changeFlag = true; }
    if (NowTime.Month != PreviousTime.Month) { changeFlag = true; }
    if (NowTime.Year != PreviousTime.Year) { changeFlag = true; }

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
        //lcd.clear();

        // First Row dd/mm/yyyy ##.#°
        lcd.setCursor(0,0);                       // Column, Row
        lcd.print(p2Digits(NowTime.Day));
        lcd.print("/");
        lcd.print(p2Digits(NowTime.Month));
        lcd.print("/");
        int i = 2000 + NowTime.Year;
        lcd.print(i);
        lcd.print(" ");
        lcd.print(String(CurrentTemperature,1));  // converts float to string
                                                  // with 1 decimal place
        lcd.print((char)223);                     // prints the degree symbol
        
        // Second Row  hh:mm dt PWSCPSN
        lcd.setCursor(0,1);                       //Column, Row
        lcd.print(p2Digits(NowTime.Hour));
        lcd.print(":");
        lcd.print(p2Digits(NowTime.Minute));
        lcd.print(" ");
        lcd.print(dow2Str(NowTime.Dow));          // Integer Day of the week
                                                  // convert to String
        lcd.print(" ");

        if (shouldShowPercent) {
          printLedStatus(percent, ledMgr.getDir());
        } else {
          printAlarmIndicators(Clock.alarmStatus(),  Clock.readAlarm(alarm1).EnabledDows, Clock.readAlarm(alarm2).EnabledDows);
        }

        PreviousTime = Clock.read();
    }
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
    }
}

void changeHour(byte i=clock0, bool increment = true){
    /*  Increments or decrements the hour by one
     *    i = 0 Clock
     *      = 1 Alarm1
     *      = 2 Alarm2
     */
    AlarmTime alarm;
    DateTime NowTime;                  //create DateTime struct from Library
    int Hour;

    switch (i){
        case clock0:
            //Clock
            NowTime = Clock.read();        // get the latest clock values
            Hour = NowTime.Hour;
            break;
        case alarm1:
            //alarm1
            alarm = Clock.readAlarm(alarm1);
            Hour = alarm.Hour;
            break;
        case alarm2:
            //alarm2
            alarm = Clock.readAlarm(alarm2);
            Hour = alarm.Hour;
            break;
        default:
            //Clock
            NowTime = Clock.read();      // get the latest clock values
            Hour = NowTime.Hour;
            break;
    }
    if (increment == true){
        Hour += 1;
    } else {
        Hour -= 1;
    }
    Hour %= 24;
    if (Hour < 0) { Hour = 23;}
    //Serial.print("24Hour = ");Serial.println(Hour);
    switch (i){
        case clock0:
            //Clock
            NowTime.Hour = byte(Hour);
            Clock.write(NowTime);
            break;
        case alarm1:
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
    }
    Serial.println("End changeHour");
    //TODO: Error checking. Would return 0 for fail and 1 for OK
}

void changeMinute(byte i=0, bool increment = true){
    /*  Increments or decrements the minute by one
     *    i = 0 Clock
     *      = 1 Alarm1
     *      = 2 Alarm2
     */
    AlarmTime alarm;
    DateTime NowTime;            //create DateTime struct from Library
    int Minute;

    switch (i){
        case clock0:
            //Clock
            NowTime = Clock.read();        // get the latest clock values
            Minute = NowTime.Minute;
            break;
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
    }
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

void changeAlarmMode(byte i=1, bool increment = true){
    /*  Change AlarmMode to 0=Daily, 1=Weekday, 2=Weekend, 3=Once
     *    i = 1 Alarm1
     *      = 2 Alarm2
     */
    if ((i==1)||(i=2)){
        // Instantiates object as struct AlarmTIme
        AlarmTime alarm = Clock.readAlarm(i);
        int AlarmMode = alarm.AlarmMode;;

        if (increment == true) {
            AlarmMode += 1;
            AlarmMode %= 4;
        } else {
            AlarmMode -= 1;
            AlarmMode %= 4;
        }
        
        if (AlarmMode < 0) { AlarmMode = 3; }
        //Serial.print("AlarmMode = ");Serial.println(AlarmMode,BIN);
        alarm.AlarmMode = byte(AlarmMode);
        Clock.setAlarm(alarm,i);
    }//TODO: Error checking. Would return 0 for fail and 1 for OK
}

void changeMonth(byte i=0, bool increment = true){
    DateTime NowTime;
    NowTime = Clock.read();
    int Month = NowTime.Month;
    if (increment == true) {
        Month += 1;
    } else {
        Month -= 1;
    }
    if (Month > 12) { Month = 1; }
    if (Month < 1) { Month = 12; }
    NowTime.Month = byte(Month);
    Clock.write(NowTime);
}

void changeDay(byte i=0, bool increment = true){
    DateTime NowTime;
    NowTime = Clock.read();
    int Day = NowTime.Day;
    byte Month = NowTime.Month;
    byte Year = NowTime.Year + 2000;
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
            if ((Year % 4 == 0) && (Year % 100 != 0) || ( Year % 400 == 0)){
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
    if (increment == true) { Day += 1; } else { Day -= 1; }
    if (Day < 1) { Day = DaysMax; }
    if (Day > DaysMax){Day = 1;}
    //Serial.print("changeDay saved = "); Serial.println(Day);
    NowTime.Day = byte(Day);
    Clock.write(NowTime);
}

void changeYear(byte i=0, bool increment = true){
    DateTime NowTime;
    NowTime = Clock.read();
    int Year = NowTime.Year;
    if (increment == true) {
        Year += 1;
    } else {
        Year -= 1;
    }
    if (Year < 18) { Year = 199; }
    if (Year > 199){ Year = 18; }
    NowTime.Year = byte(Year);
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

void toggleLED(bool ledON = true){
    bool ledState;

    ledState = digitalRead(LED_Pin);                //get the state of LED
    if (ledON == true) {
        digitalWrite(LED_Pin, !ledState);           //do the opposite
    } else {
        digitalWrite(LED_Pin, LOW);
    }
}


void clearAlarms(void){
    //Clear alarm flags
    Clock.clearAlarms();
    toggleLED(false);
    lcd.display();                     // Just in case it was off
}

void editClock(byte i=0){
    //First  Row  mm/dd/yyyy ##.#°
    //Second Row  hh:mm AM DT PWSCPSN
    byte cursorPositions[][2] = {{1,0},{4,0},{9,0},{1,1},{4,1}};
    //lcd.setCursor(Column, Row);
    //Serial.print("editclock position = "); Serial.println(i);
    lcd.setCursor(cursorPositions[i][0],cursorPositions[i][1]);
    lcd.cursor();
    lcd.blink();
}

void editAlarm(byte i=0){
    /* Alarm 1      ON
       hh:mm    PWSCPSN*/
    //Note valid i values are 0-3
    //lcd.setCursor(Column, Row);
    byte cursorPositions[][2] = {{1,1},{4,1},{9,1},{10,1},{11,1},{12,1},{13,1},{14,1},{15,1}};
    //Serial.print("editAlarm position = ");Serial.println(i);
    lcd.setCursor(cursorPositions[i][0],cursorPositions[i][1]);
    lcd.cursor();
    lcd.blink();
}

void ButtonClick(Button& b){
    //Clocks
    //const byte clock0=0;
    //const byte alarm1=1;
    //const byte alarm2=2;

    //Debug code to Serial monitor
    Serial.print("Button Click - ");
    switch(b.pinValue()){
        case Mode_Pin:
            Serial.println("Mode_Pin");
            break;
        case Lt_Pin:
            Serial.println("Lt_Pin");
            break;
        case Rt_Pin:
            Serial.println("Rt_Pin");
            break;
        case Switch_Pin:
            Serial.println("Switch_Pin");
            break;
        default:
            //do nothing
            break;
    }

    if (bHoldButtonFlag == true) {
        // After a hold button is released, a button click is also registered
        if (b.pinValue() == Switch_Pin) {
            ledMgr.finishSettingMaxValue();
            uint16_t maxLedValue = ledMgr.getMaxValue();
            EEPROM.write(EEPROM_ADDR_MAX_LED_LEVEL,maxLedValue);
            EEPROM.write(EEPROM_ADDR_MAX_LED_LEVEL + 1, maxLedValue >> 8);
            EEPROM.commit();
            Serial.print("Saved max alarm LED light value at: ");
            Serial.println(maxLedValue);
        } else {
            // ignore clicks for SkipClickTime ms
            // if ((millis() - buttonHoldPrevTime) > SkipClickTime) { bHoldButtonFlag = false;}
            Serial.println("Button Click ignored");
        }
        bHoldButtonFlag = false;
    } else {
        //PowerLoss,ShowClock, Alarm, EditClock, EditAlarm1, EditAlarm2
        byte alarm = alarm2;
        bool shouldIncrement = true;
        byte dowToChange=0;

        switch (ClockState){
            case PowerLoss:
                //any clickbutton and return to ShowClock
                ClockState = ShowClock;
                Clock.clearOSFStatus();
                break;
            case ShowClock:
                //ShowClock Mode
                //show alarm screens
                switch (b.pinValue()){
                    case Mode_Pin:
                        //Do Nothing
                        break;
                    case Lt_Pin:
                        toggleShowAlarm(alarm1);
                        break;
                    case Rt_Pin:
                        toggleShowAlarm(alarm2);
                        break;
                    default:
                        //do nothing
                        break;
                }
                break;
            //ShowAlarm1 or ShowAlarm2 does nothing
            case EditClock:
                //Edit Clock Mode
                switch (b.pinValue()){
                    case Mode_Pin:
                        //Increments cursor position
                        //cpIndex += 1 % 7;
                        cpIndex += 1;
                        cpIndex %= 5;
                        break;
                    case Lt_Pin:
                        shouldIncrement = false;
                    case Rt_Pin:
                        // First  Row dd/mm/yyyy ##.#°
                        //             0  1    2
                        // Second Row hh:mm dt PWSCPSN
                        //             3  4
                        switch (cpIndex){
                            case 0:
                                //edit day
                                changeDay(clock0, shouldIncrement);
                                break;
                            case 1:
                                //edit month
                                changeMonth(clock0, shouldIncrement);
                                break;
                            case 2:
                                //edit year
                                changeYear(clock0, shouldIncrement);
                                break;
                            case 3:
                                //edit Hours
                                changeHour(clock0, shouldIncrement);
                                break;
                            case 4:
                                //edit Minute
                                changeMinute(clock0, shouldIncrement);
                                break;
                            default:
                                //do nothing
                                break;
                        }
                        break;
                    default:
                        //do nothing
                        break;
                }
                //End EditClock
                break;
            case EditAlarm1:
                //Edit Alarm1
                alarm = alarm1;
            case EditAlarm2:
                //Edit Alarm2
                switch (b.pinValue()){
                    case Mode_Pin:
                        //Increments cursor position
                        cpIndex += 1;
                        cpIndex %= 9;
                        break;
                    case Lt_Pin:
                        // Decrements value 
                        shouldIncrement = false;
                    case Rt_Pin:
                        // Increments value
                        // Second Row hh:mm PWSCPSN
                        //             0  1 2345678 
                        switch (cpIndex){
                            case 0:
                                //edit Hours
                                changeHour(alarm, shouldIncrement);
                                break;
                            case 1:
                                //edit Minute
                                changeMinute(alarm, shouldIncrement);
                                break;
                            case 2:
                            case 3:
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                            case 8:
                                dowToChange = (cpIndex == 8)? 1 : cpIndex; // Nd is on last position but clock library assumes Sun is on 1-st position: Nd (8) -> Sun (1)
                                changeEnabledDows(alarm, dowToChange);
                                break;
                            default:
                                //do nothing
                                break;
                        }
                        break;
                    default:
                        //do nothing
                        break;
                }
                break;
            default:
                //todo
                break;
        }
        if (b.pinValue() == Switch_Pin) {
          ledMgr.handleSwitch();
          displayClock(true);
        }
    }
}

void ButtonHold(Button& b){
    //Clock States:
    // PowerLoss, ShowClock, ShowAlarm1, ShowAlarm2, Alarm, EditClock, EditAlarm1, EditAlarm2
    // static unsigned long buttonHoldPrevTime = 0;

    //Debug code to Serial monitor
    Serial.print("Button Hold - ");
    switch(b.pinValue()){
        case Mode_Pin:
            Serial.println("Mode_Pin");
            break;
        case Lt_Pin:
            Serial.println("Lt_Pin");
            break;
        case Rt_Pin:
            Serial.println("Rt_Pin");
            break;
        case Switch_Pin:
            Serial.println("Switch_Pin");
            break;
        default:
            //do nothing
            break;
    }

    // To ignore back to back button hold? 
    if ((millis()-buttonHoldPrevTime) > 2000){
        switch (ClockState){
            case PowerLoss:
                //Any button held
                //Edit main clock display
                ClockState = EditClock;
                cpIndex = 0;
                buttonHoldPrevTime = millis();
                bHoldButtonFlag = true;
                Clock.clearOSFStatus();
                break;
            case ShowClock:
                switch (b.pinValue()){
                    case Mode_Pin:
                        //Edit main clock display
                        ClockState = EditClock;
                        cpIndex = 0;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        break;
                    case Lt_Pin:
                        //Edit Alarm1
                        ClockState = EditAlarm1;
                        cpIndex = 0;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        displayAlarm(1,true);
                        break;
                    case Rt_Pin:
                        //Edit Alarm2
                        ClockState = EditAlarm2;
                        cpIndex = 0;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        displayAlarm(2,true);
                        break;
                    default:
                        break;
                }
                break;
            case ShowAlarm1:
                switch (b.pinValue()){
                    case Mode_Pin:
                        break;
                    case Lt_Pin:
                        ClockState = EditAlarm1;
                        cpIndex = 0;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        displayAlarm(1,true);
                        //Switch to edit mode
                        break;
                    case Rt_Pin:
                        //Do Nothing
                        break;
                    default:
                        break;
                }
                break;
            case ShowAlarm2:
                switch (b.pinValue()){
                    case Mode_Pin:
                        break;
                    case Lt_Pin:
                        break;
                    case Rt_Pin:
                        //Edit Alarm2
                        ClockState = EditAlarm2;
                        cpIndex = 0;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        displayAlarm(2,true);
                        break;
                    default:
                        break;
                }
                break;
            case EditClock:  //Edit Clock
                switch (b.pinValue()){
                    case Mode_Pin:
                        lcd.noBlink();
                        lcd.noCursor();
                        ClockState = ShowClock;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        break;
                    case Lt_Pin:
                    case Rt_Pin:
                    default:
                        break;
                }
                break;
            case EditAlarm1:  //Edit Alarm1
                switch (b.pinValue()){
                    case Mode_Pin:
                        lcd.noBlink();
                        lcd.noCursor();
                        ClockState = ShowClock;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        displayClock(true);
                        break;
                    case Lt_Pin:
                    case Rt_Pin:
                    default:
                        break;
                }
                break;
            case EditAlarm2:  //Edit Alarm1
                switch (b.pinValue()){
                    case Mode_Pin:
                        lcd.noBlink();
                        lcd.noCursor();
                        ClockState = ShowClock;
                        buttonHoldPrevTime = millis();
                        bHoldButtonFlag = true;
                        displayClock(true);
                        break;
                    case Lt_Pin:
                    case Rt_Pin:
                    default:
                        break;
                }
                break;
            default:
                //todo
                break;
        }

        if (b.pinValue() == Switch_Pin) {
            ledMgr.beginSettingMaxValue();
            Serial.println("Setting max alarm LED light value started...");
            bHoldButtonFlag = true;
        }
    }
}

String dow2Str(byte bDow) {
    // Day of week to string or char array. DOW 1=Sunday, 0 is undefined
    static const char *str[] = {"---", "Nd", "Pn", "Wt", "Sr", "Cz", "Pt", "So"};
    if (bDow > 7) bDow = 0;
    return(str[bDow]);
}

String p2Digits(int numValue) {
    // utility function for digital clock display
    // converts int to two digit char array
    String str;

    if(numValue < 10) {
        str = "0" + String(numValue);
    } else {
        str = String(numValue);
    }
    return str;
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
 *                         Void Setup                        *
 * ********************************************************* */
void setup() {
    // Get the start time
    RunTime = millis();

    //Serial Monitor
    Serial.begin(500000);
    delay(500);
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

    pinMode(LED_Pin, OUTPUT);
    digitalWrite(LED_Pin, HIGH);
    pinMode(SQW_Pin, INPUT);

    byte maxLedValueLow = EEPROM.read(EEPROM_ADDR_MAX_LED_LEVEL);
    byte maxLedValueHigh = EEPROM.read(EEPROM_ADDR_MAX_LED_LEVEL + 1);
    uint16_t maxLedValue = ((uint16_t)maxLedValueHigh << 8) + (uint16_t)maxLedValueLow;
    Serial.print("Max Alarm LED value: ");
    Serial.println(maxLedValue);
    ledMgr.setMaxValue(maxLedValue);

    /*          LCD Stuff           */
    lcd.init();                      // initialize the lcd 

    /*         Clock Stuff          */
    Clock.begin();
    //Clock.setInterruptCtrl(true);
    if (Clock.getOSFStatus() == true){
        //Restart from power loss detected
        ClockState = PowerLoss;
        Serial.println("PowerLoss State");
    }
    CurrentTemperature = getTemperatureValue();

    /*  Button callback functions   */
    LtKey.clickHandler(ButtonClick);
    LtKey.holdHandler(ButtonHold,Button_Hold_Time);
    RtKey.clickHandler(ButtonClick);
    RtKey.holdHandler(ButtonHold,Button_Hold_Time);
    ModeKey.clickHandler(ButtonClick);
    ModeKey.holdHandler(ButtonHold,Button_Hold_Time);
    SwitchKey.clickHandler(ButtonClick);
    SwitchKey.holdHandler(ButtonHold,Button_Hold_Time);

    //Display the clock
    displayClock(true);

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

    //Debug code
    Serial.print("Register 0x0E = ");Serial.println(Clock.getCtrlRegister(), BIN);
    Serial.print("Register 0x0F = ");Serial.println(Clock.getStatusRegister(), BIN);
    Serial.println("Setup End");
}

/* ***********************************************************
 *                         Void Loop                         *
 * ********************************************************* */
void loop() {
    static unsigned long previousLcdMillis = 0;
    unsigned long mills = millis();
    //if (ClockState != PrevState) { Serial.print("ClockState = ");Serial.println(ClockState); PrevState = ClockState;}

    switch (ClockState){
        case PowerLoss:
            if (ClockState != PrevState) { Serial.println("ClockState = PowerLoss"); PrevState = ClockState;}
            //Serial.println("PowerLoss");
            displayClock();
            //Flash Clock
            if ((mills-previousLcdMillis) >= flashInterval) {
                previousLcdMillis = mills;
                if (bDisplayStatus == true){
                    lcd.noDisplay();
                } else {
                    lcd.display();
                }
                bDisplayStatus = !bDisplayStatus;
            }
            break;
        case ShowClock:
            if (ClockState != PrevState) { Serial.println("ClockState = ShowClock"); PrevState = ClockState;}
            //Serial.println("ShowClock");
            //lcd.display();                     // Just in case it was off
            displayClock();
            break;
        case ShowAlarm1:
            if (ClockState != PrevState) { Serial.println("ClockState = ShowAlarm1"); PrevState = ClockState;}
            //AlarmRunTime is defined by toggleShowAlarm
            if ((mills-AlarmRunTime) <= Alarm_View_Pause) {
                displayAlarm(alarm1);
            } else {
                ClockState = ShowClock;
                displayClock(true);
            }
            break;
        case ShowAlarm2:
            if (ClockState != PrevState) { Serial.println("ClockState = ShowAlarm2"); PrevState = ClockState;}
            //AlarmRunTime is defined by toggleShowAlarm
            if ((mills-AlarmRunTime) <= Alarm_View_Pause) {
                displayAlarm(alarm2);
            } else {
                ClockState = ShowClock;
                displayClock(true);
            }
            break;
        case EditClock:
            if (ClockState != PrevState) { Serial.println("ClockState = EditClock"); PrevState = ClockState;}
            editClock(cpIndex);
            displayClock();
            break;
        case EditAlarm1:
            //Edit Alarm1
            if (ClockState != PrevState) { Serial.println("ClockState = EditAlarm1"); PrevState = ClockState;}
            editAlarm(cpIndex);
            displayAlarm(alarm1);
            break;
        case EditAlarm2:
            //Edit Alarm2
            if (ClockState != PrevState) { Serial.println("ClockState = EditAlarm2"); PrevState = ClockState;}
            editAlarm(cpIndex);
            displayAlarm(alarm2);
            break;
        default:
            Serial.println("ClockState = default!!");
            displayClock();
            break;
    }

    bool shouldMoveOn = ledMgr.shouldMoveOn();
    if (shouldMoveOn) {
      shouldShowPercent = true;
    } else {
      if (prevShouldMoveOn) {
        displayClock(true);
        shouldShowPercent = false;
        log_d("Max Level reached at %d%%", ledMgr.getPercent());
      }
    }
    prevShouldMoveOn = shouldMoveOn;


    LtKey.process();
    RtKey.process();
    ModeKey.process();
    SwitchKey.process();
    byte activeAlarms = CheckAlarmStatus();  //Returns which alarms are activated    
    if (activeAlarms) {
        Serial.print("Active alarms: ");
        Serial.println(activeAlarms);
        ledMgr.setDirAndProfile(1, LightProfileName::Alarm);
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
      }

      delay(2);
    }
}
