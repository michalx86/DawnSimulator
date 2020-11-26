#ifndef Lcd_h
#define Lcd_h


class Lcd {
public:
  size_t print(String str) {return 0;}
  size_t print(int i) {return 0;}
  size_t print(char i) {return 0;}
  size_t write(byte i) {return 0;}

  char content[2][16]  = {{0},{0}};
};

#endif // Lcd_h
