#include <Esp.h>+
#include "LedStripMgr.h"

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

const LightProfile alarmLightProfile(LightProfileName::Alarm);
const LightProfile switchLightProfile(LightProfileName::Switch);
LightState alarmLightState(alarmLightProfile);
LightState switchLightState(switchLightProfile);

LedStripMgr::LedStripMgr(int r_pin, int g_pin, int b_pin, int ww_pin, int cw_pin): LED_Pins{r_pin, g_pin, b_pin, ww_pin,cw_pin}, lightState(&alarmLightState) {
}

void LedStripMgr::init() {
  for (int i = 0; i < LED_LAST; i++) {
    ledcSetup(LEDC_CHANNEL_0 + i, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(LED_Pins[LED_R + i], LEDC_CHANNEL_0 + i);
    ledWrite((LED_COLOR)i, 0);
  }
}

bool LedStripMgr::shouldMoveOn() {
  bool retVal = false;
  portENTER_CRITICAL(&mux);
  retVal = (((stepDir ==  1) && (level < lightState->getTargetLevel())) ||
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
  retVal = lightState->getTargetLevelValue();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setTargetLevelFromValue(uint16_t value) {
  portENTER_CRITICAL(&mux);
  alarmLightState.setTargetLevelFromValue(value);
  switchLightState.setTargetLevelFromValue(value);

  unsigned targetLevel = lightState->getTargetLevel();
  Serial.print("Target LED level: ");
  Serial.println(targetLevel);
  portEXIT_CRITICAL(&mux);
}

int LedStripMgr::getPercent() {
  int retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = lightState->toPercent(level);
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setDirAndProfile(int dir, LightProfileName profileName) {
  switch (profileName) {
    case LightProfileName::Alarm  : setDirAndLightState(dir, alarmLightState); break;
    case LightProfileName::Switch : setDirAndLightState(dir, switchLightState); break;
    default : break;
  }
}

void LedStripMgr::beginSettingTargetLevel() {
  portENTER_CRITICAL(&mux);
  stepDir = 1;
  level = 0;
  lightState = &switchLightState;
  lightState->resetTargetLevel();
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::finishSettingTargetLevel() {
  portENTER_CRITICAL(&mux);
  stepDir = 0;
  lightState->setTargetLevel(level);
  // TODO: When setting target values is separated, this should be removed
  unsigned value = (*lightState)[level];
  setTargetLevelFromValue(value);
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::handlSwitch() {
  portENTER_CRITICAL(&mux);
  int newDir = (stepDir != 0)? -stepDir : (level == 0)? 1 : -1;
  setDirAndLightState(newDir, switchLightState);
  portEXIT_CRITICAL(&mux);
}

bool LedStripMgr::changeLight(unsigned long timeSinceLastLightChange) {
  bool retVal = false;
  portENTER_CRITICAL(&mux);

  if (timeSinceLastLightChange > lightState->getSampleDuration()) {
    if (stepDir != 0) {
      if (shouldMoveOn()) {
        level += stepDir;
        ledWrite(LED_COLOR(4), (*lightState)[level]);
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
void LedStripMgr::ledWrite(LED_COLOR color, unsigned val) {
  static unsigned last_led_ww_value = 255;

  if (val != last_led_ww_value) {
    last_led_ww_value = val;
    ledcWrite(LEDC_CHANNEL_0 + color, last_led_ww_value);
  }
}

void LedStripMgr::setDirAndLightState(int dir, LightState &state) {
  portENTER_CRITICAL(&mux);
  stepDir = dir;
  if (lightState != &state) {
    level       = state.sampleHigherOrEqual((*lightState)[level]);
    lightState = &state;
  }
  portEXIT_CRITICAL(&mux);
}
