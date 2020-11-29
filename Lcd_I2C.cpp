#include "Lcd_I2C.h"

    //custom LCD characters: https://omerk.github.io/lcdchargen/
    //Alarm 1 indicator
    byte cA1[8] = {
                  0b00100,
                  0b01010,
                  0b10001,
                  0b11111,
                  0b00100,
                  0b00000,
                  0b00000,
                  0b00000 };
    //Alarm 2 indicator
    byte cA2[8] = {
                  0b00000,
                  0b00000,
                  0b00000,
                  0b00100,
                  0b01010,
                  0b10001,
                  0b11111,
                  0b00100 };
    //Both arrows indicator
    byte cBA[8] = {
                  0b00100,
                  0b01010,
                  0b10001,
                  0b11111,
                  0b01010,
                  0b10001,
                  0b11111,
                  0b00100 };
    //Up Arrow
    byte cUpArrow[8] = {
                    0b00001,
                    0b00011,
                    0b00111,
                    0b01111,
                    0b10111,
                    0b00101,
                    0b01100,
                    0b11000 };
    //Downd Arrow
    byte cDownArrow[8] = {
                    0b11000,
                    0b01100,
                    0b00101,
                    0b10111,
                    0b01111,
                    0b00111,
                    0b00011,
                    0b00001 };



Lcd_I2C::Lcd_I2C() : lcd(0x27,20,4) {  // set the LCD address to 0x27 for a 16 chars and 2 line display
}

void Lcd_I2C::init() {
    lcd.init();                      // initialize the lcd 
    lcd.begin(16, 2);

    //Create custom lcd characters
    lcd.createChar(LCD_CHAR_ALARM1, cA1);
    lcd.createChar(LCD_CHAR_ALARM2, cA2);
    lcd.createChar(LCD_CHAR_BOTH_ALARMS, cBA);
    lcd.createChar(LCD_CHAR_UP_ARROW, cUpArrow);
    lcd.createChar(LCD_CHAR_DOWN_ARROW, cDownArrow);

    lcd.backlight();
}

  void Lcd_I2C::clear() {
    lcd.clear();
  }
  void Lcd_I2C::noDisplay() {
    lcd.noDisplay();
  }
  void Lcd_I2C::display() {
    lcd.display();
  }
  void Lcd_I2C::noBlink() {
    lcd.noBlink();
  }
  void Lcd_I2C::blink() {
    lcd.blink();
  }
  void Lcd_I2C::noCursor() {
    lcd.noCursor();
  }
  void Lcd_I2C::cursor() {
    lcd.cursor();
  }
  void Lcd_I2C::createChar(uint8_t location, uint8_t charmap[]) {
    lcd.createChar(location, charmap);
  }
  void Lcd_I2C::setCursor(uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
  } 


inline size_t Lcd_I2C::write(uint8_t value) {
  return lcd.write(value);
}
