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
  void setMaxValue(Color_t value);
  int getPercent();

  void setDirAndProfile(int dir, LightProfileName profileName);
  void beginSettingMaxValue();
  void finishSettingMaxValue();
  void handleSwitch();

  bool changeLight(unsigned long timeSinceLastLightChange);

private:
  void ledWrite(LED_COLOR color, unsigned val);
  void setDirAndLightComposite(int dir, LightComposite &profile);
  void diagnostic();

  int LED_Pins[LED_LAST];
  LightComposite* lightComposite;
  int stepDir = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
