#include "LightProfile.h"

class LedStripMgr {
public:
  LedStripMgr(int pin);
  void init();
  bool shouldMoveOn();
  int getDir();
  void setDir(int dir);
  unsigned getLevel();
  unsigned getTargetLevelValue();
  void setTargetLevelFromValue(unsigned value);
  int getPercent();
  int getTargetPercent();
  void setShouldDimm(bool dimm);

  void setDirAndProfile(int dir, LightProfileName profileName);
  void beginSettingTargetLevel();
  void finishSettingTargetLevel();
  void handlSwitch();

  bool changeLight(unsigned long timeSinceLastLightChange);

private:
  void ledWwWrite(unsigned val);
  void setDirAndProfile(int dir, const LightProfile &profile);

  const int WW_Pin = 0;
  const LightProfile* lightProfile;
  unsigned level = 0;
  unsigned targetLevel = 0;
  int stepDir = 0;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
