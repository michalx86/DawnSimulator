#include "LightState.h"

class LedStripMgr {
public:
  LedStripMgr(int pin);
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
  void ledWwWrite(unsigned val);
  void setDirAndLightState(int dir, LightState &profile);

  const int WW_Pin = 0;
  LightState* lightState;
  unsigned level = 0;
  int stepDir = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
