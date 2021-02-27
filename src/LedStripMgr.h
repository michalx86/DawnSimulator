#include "LightComposite.h"



class LedStripMgr {
public:
  LedStripMgr(int r_pin, int g_pin, int b_pin, int ww_pin, int cw_pin);
  void init();
  bool shouldMoveOn();
  int getDir();
  void setDir(int dir);
  unsigned getLevel();
  Color_t getMaxValue();
  int getPercent();

  void handleLightOn(LightProfileName profileName, Color_t toColor);
  void handleLightOff(LightProfileName profileName);

  bool changeLight(unsigned long timeSinceLastLightChange);

private:
  LightComposite* profileName2Composite(LightProfileName profileName);
  void ledWrite(LED_COLOR color, unsigned val);
  void diagnostic();

  int LED_Pins[LED_LAST];
  LightComposite* lightComposite;
  int stepDir = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
