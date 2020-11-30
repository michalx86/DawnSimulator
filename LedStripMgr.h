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
  void setDirAndProfile(int dir, LightProfileName profileName);
  int getPercent();
  int getTargetPercent();
  void beginSettingTargetLevel();
  void finishSettingTargetLevel();
  void handlSwitch();
  void ledWwWrite(unsigned val);
  void setShouldDimm(bool dimm);
  bool changeLight(unsigned long timeSinceLastLightChange);

private:
  const int Led_WW_Pin = 0;
  LightProfile &lightProfile;
  unsigned ledLevel = 0;
  unsigned targetLedLevel = 0;
  int ledStepDir = 0;
  bool shouldDimm = false;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};
