#include "LightState.h"

enum LED_COLOR {
  LED_R = 0, LED_G, LED_B, LED_WW, LED_CW, LED_LAST
};



class LedStripMgr {
public:
  LedStripMgr(int r_pin, int g_pin, int b_pin, int ww_pin, int cw_pin);
  void init();
  bool shouldMoveOn();
  int getDir();
  void setDir(int dir);
  unsigned getLevel();
  unsigned getTargetLevelValue();
  void setTargetLevelFromValue(uint16_t value);
  int getPercent();
  void setShouldDimm(bool dimm);

  void setDirAndProfile(int dir, LightProfileName profileName);
  void beginSettingTargetLevel();
  void finishSettingTargetLevel();
  void handlSwitch();

  bool changeLight(unsigned long timeSinceLastLightChange);

private:
  void ledWrite(LED_COLOR color, unsigned val);
  void setDirAndLightState(int dir, LightState &profile);

  int LED_Pins[LED_LAST];
  LightState* lightState;
  unsigned level = 0;
  int stepDir = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
