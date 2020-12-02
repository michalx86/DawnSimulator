#include <Esp.h>
#include "LedStripMgr.h"

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

const LightProfile alarmLightProfile(LightProfileName::Alarm);
const LightProfile switchLightProfile(LightProfileName::Switch);

LedStripMgr::LedStripMgr(int pin): WW_Pin(pin), lightProfile(&switchLightProfile) {
}

void LedStripMgr::init() {
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(WW_Pin, LEDC_CHANNEL_0);
  ledWwWrite(0);
}

bool LedStripMgr::shouldMoveOn() {
  bool retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = (((stepDir ==  1) && (level < targetLevel)) ||
          ((stepDir == -1) && (level > 0)));
  portEXIT_CRITICAL(&mux);
  return retVal;
}

int LedStripMgr::getDir() {
  int retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = stepDir;
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setDir(int dir) {
  portENTER_CRITICAL(&mux);
  stepDir = dir;
  portEXIT_CRITICAL(&mux);
}

unsigned LedStripMgr::getLevel() {
  unsigned retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = level;
  portEXIT_CRITICAL(&mux);
  return retVal;
}

unsigned LedStripMgr::getTargetLevelValue() {
  unsigned retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = (*lightProfile)[targetLevel];
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setTargetLevelFromValue(unsigned value) {
  portENTER_CRITICAL(&mux);
  targetLevel = lightProfile->sampleHigherOrEqual(value);
  Serial.print("Target LED level: ");
  Serial.println(targetLevel);
  portEXIT_CRITICAL(&mux);
}

int LedStripMgr::getPercent() {
  int retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = lightProfile->toPercent(level);
  portEXIT_CRITICAL(&mux);
  return retVal;
}

int LedStripMgr::getTargetPercent() {
  int retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = lightProfile->toPercent(targetLevel);
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setDirAndProfile(int dir, LightProfileName profileName) {
  const LightProfile* profile = nullptr;
   switch (profileName) {
    case LightProfileName::Alarm  : profile = &alarmLightProfile; break;
    case LightProfileName::Switch : profile = &switchLightProfile; break;
    default : break;
  }
  setDirAndProfile(dir, *profile);
}

void LedStripMgr::beginSettingTargetLevel() {
  portENTER_CRITICAL(&mux);
  stepDir = 1;
  level = 0;
  lightProfile = &switchLightProfile;
  targetLevel = lightProfile->samplesNum() - 1;
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::finishSettingTargetLevel() {
  portENTER_CRITICAL(&mux);
  stepDir = 0;
  targetLevel = level;
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::handlSwitch() {
  portENTER_CRITICAL(&mux);
  int newDir = (stepDir != 0)? -stepDir : (level == 0)? 1 : -1;
  setDirAndProfile(newDir, switchLightProfile);
  portEXIT_CRITICAL(&mux);
}

bool LedStripMgr::changeLight(unsigned long timeSinceLastLightChange) {
  bool retVal = false;
  portENTER_CRITICAL(&mux);

  if (timeSinceLastLightChange > lightProfile->getSampleDuration()) {
    if (stepDir != 0) {
      if (shouldMoveOn()) {
        level += stepDir;
        ledWwWrite((*lightProfile)[level]);
        retVal = true;
      }
      if (shouldMoveOn() == false) {
        log_d("l: %u, d: %d",level, stepDir);
        stepDir = 0;
      }
    }
  }
  portEXIT_CRITICAL(&mux);
  return retVal;
}

/*###############################################################
  # Private
  ###############################################################*/
void LedStripMgr::ledWwWrite(unsigned val) {
  static unsigned last_led_ww_value = 255;

  if (val != last_led_ww_value) {
    last_led_ww_value = val;
    ledcWrite(LEDC_CHANNEL_0, last_led_ww_value);
  }
}

void LedStripMgr::setDirAndProfile(int dir, const LightProfile &profile) {
  portENTER_CRITICAL(&mux);
  stepDir = dir;
  if (lightProfile != &profile) {
    level       = profile.sampleHigherOrEqual((*lightProfile)[level]);
    targetLevel = profile.sampleHigherOrEqual((*lightProfile)[targetLevel]);
    lightProfile = &profile;
  }
  portEXIT_CRITICAL(&mux);
}
