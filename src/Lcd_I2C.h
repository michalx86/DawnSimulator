#ifndef Lcd_I2C_h
#define Lcd_I2C_h

#include <Esp.h>
#include <LiquidCrystal_I2C.h>
#include "Print.h" 

const byte LCD_CHAR_ALARM1      = 1;
const byte LCD_CHAR_ALARM2      = 2;
const byte LCD_CHAR_BOTH_ALARMS = 3;
const byte LCD_CHAR_UP_ARROW    = 4;
const byte LCD_CHAR_DOWN_ARROW  = 5;



class Lcd_I2C : public Print {
public:
  Lcd_I2C();
  void init();
  void clear();
  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 

  virtual size_t write(uint8_t);
private:
  LiquidCrystal_I2C lcd;
};

#endif // Lcd_I2C_h
