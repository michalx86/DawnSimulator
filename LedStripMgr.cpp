#include <Esp.h>
#include "LedStripMgr.h"

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

LightProfile alarmLightProfile(LightProfileName::Alarm);
LightProfile switchLightProfile(LightProfileName::Switch);

bool LedStripMgr::shouldMoveOn() {
  bool retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = (((ledStepDir ==  1) && (ledLevel < targetLedLevel)) ||
          ((ledStepDir == -1) && (ledLevel > 0)));
  portEXIT_CRITICAL(&mux);
  return retVal;
}

int LedStripMgr::getDir() {
  int retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = ledStepDir;
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setDir(int dir) {
  portENTER_CRITICAL(&mux);
  ledStepDir = dir;
  portEXIT_CRITICAL(&mux);
}

unsigned LedStripMgr::getLevel() {
  unsigned retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = ledLevel;
  portEXIT_CRITICAL(&mux);
  return retVal;
}

unsigned LedStripMgr::getTargetLevelValue() {
  unsigned retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = lightProfile[targetLedLevel];
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setTargetLevelFromValue(unsigned value) {
  portENTER_CRITICAL(&mux);
  targetLedLevel = lightProfile.sampleHigherOrEqual(value);
  Serial.print("Target LED level: ");
  Serial.println(targetLedLevel);
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::setDirAndProfile(int dir, LightProfileName profileName) {
  portENTER_CRITICAL(&mux);
  ledStepDir = dir;
   switch (profileName) {
    case LightProfileName::Alarm  : lightProfile = alarmLightProfile; break;
    case LightProfileName::Switch : lightProfile = switchLightProfile; break;
    default : break;
  }
  portEXIT_CRITICAL(&mux);
}

int LedStripMgr::getPercent() { 
  int retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = lightProfile.toPercent(ledLevel);
  portEXIT_CRITICAL(&mux);
  return retVal;
}

int LedStripMgr::getTargetPercent() { 
  int retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = lightProfile.toPercent(targetLedLevel); 
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::beginSettingTargetLevel() {
  portENTER_CRITICAL(&mux);
  ledStepDir = 1;
  ledLevel = 0;
  targetLedLevel = lightProfile.samplesNum() - 1;
  lightProfile = switchLightProfile;
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::finishSettingTargetLevel() {
  portENTER_CRITICAL(&mux);
  ledStepDir = 0;
  targetLedLevel = ledLevel;
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::handlSwitch() {
  portENTER_CRITICAL(&mux);
  if (ledStepDir != 0) {
    ledStepDir *= -1;
  } else if (ledLevel == 0) {
    ledStepDir = 1;
  } else {
    ledStepDir = -1;
  }
  lightProfile = switchLightProfile;
  portEXIT_CRITICAL(&mux);
}


void LedStripMgr::ledWwWrite(unsigned val) {
  portENTER_CRITICAL(&mux);
  static unsigned last_led_ww_value = 255;

  if (val != last_led_ww_value) {
    last_led_ww_value = val;  
    ledcWrite(LEDC_CHANNEL_0, last_led_ww_value);
  }
  portEXIT_CRITICAL(&mux);
}


bool LedStripMgr::changeLight(unsigned long timeSinceLastLightChange ) {
  bool retVal = false;
  portENTER_CRITICAL(&mux);

  if ((ledStepDir != 0) && (timeSinceLastLightChange > lightProfile.getSampleDuration())) {
    if (shouldMoveOn()) {
      ledLevel += ledStepDir;
      ledWwWrite(lightProfile[ledLevel]);
    } else {
      ledStepDir = 0;
    }
    retVal = true;
  }
  portEXIT_CRITICAL(&mux);
  return retVal;
}

LedStripMgr::LedStripMgr(int pin): Led_WW_Pin(pin), lightProfile(switchLightProfile) {
}

void LedStripMgr::init() {
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(Led_WW_Pin, LEDC_CHANNEL_0);
  ledWwWrite(0);
}
