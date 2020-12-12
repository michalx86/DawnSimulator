#include <Esp.h>+
#include "LedStripMgr.h"

const unsigned ALARM_LIGHTENING_PERIOD_MS = 20 * 60 * 1000;
const unsigned SWITCH_LIGHTENING_PERIOD_MS = 2000;

// use 500 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     500



// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

const LightProfile alarmLightProfile(LightProfileName::Alarm);
const LightProfile switchLightProfile(LightProfileName::Switch);
LightComposite alarmLightComposite(alarmLightProfile, ALARM_LIGHTENING_PERIOD_MS);
LightComposite switchLightComposite(switchLightProfile, SWITCH_LIGHTENING_PERIOD_MS);

LedStripMgr::LedStripMgr(int r_pin, int g_pin, int b_pin, int ww_pin, int cw_pin): LED_Pins{r_pin, g_pin, b_pin, ww_pin,cw_pin}, lightComposite(&alarmLightComposite) {
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
  retVal = (((stepDir ==  1) && (level < lightComposite->lastSampleNum())) ||
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

unsigned LedStripMgr::getTargetValue() {
  unsigned retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = lightComposite->getTargetValue();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setTargetValue(uint16_t value) {
  portENTER_CRITICAL(&mux);
  alarmLightComposite.setTargetValue(value);
  switchLightComposite.setTargetValue(value);
  portEXIT_CRITICAL(&mux);
}

int LedStripMgr::getPercent() {
  int retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = lightComposite->getPercent();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setDirAndProfile(int dir, LightProfileName profileName) {
  switch (profileName) {
    case LightProfileName::Alarm  : setDirAndLightComposite(dir, alarmLightComposite); break;
    case LightProfileName::Switch : setDirAndLightComposite(dir, switchLightComposite); break;
    default : break;
  }
}

void LedStripMgr::beginSettingTargetValue() {
  portENTER_CRITICAL(&mux);
  stepDir = 1;
  level = 0;
  lightComposite = &switchLightComposite;
  lightComposite->resetTargetValue();
  lightComposite->setCurrentValue(0);
  lightComposite->setSourceValue(0);
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::finishSettingTargetValue() {
  portENTER_CRITICAL(&mux);
  stepDir = 0;
  // TODO: When setting target values is separated, this should be removed
  unsigned value = (*lightComposite)[level];
  setTargetValue(value);
  level = lightComposite->lastSampleNum();
  portEXIT_CRITICAL(&mux);
}
/*
int cnt = 0;
void LedStripMgr::handleSwitch() {
  portENTER_CRITICAL(&mux);
  int newDir = (stepDir != 0)? -stepDir : (level == 0)? 1 : -1;
  if (cnt++ % 2 == 0) {
    setDirAndLightComposite(newDir, switchLightComposite);
  } else {
    setDirAndLightComposite(newDir, alarmLightComposite);
  }
  portEXIT_CRITICAL(&mux);
}*/

void LedStripMgr::handleSwitch() {
  portENTER_CRITICAL(&mux);
  int newDir = (stepDir != 0)? -stepDir : (level == 0)? 1 : -1;
  setDirAndLightComposite(newDir, switchLightComposite);
  portEXIT_CRITICAL(&mux);
}

bool LedStripMgr::changeLight(unsigned long timeSinceLastLightChange) {
  bool retVal = false;
  portENTER_CRITICAL(&mux);

  if (timeSinceLastLightChange > lightComposite->getSampleDuration()) {
    if (stepDir != 0) {
      if (shouldMoveOn()) {
        level += stepDir;
        int profile_val = (*lightComposite)[level];
        int source_val = lightComposite->getSourceValue();
        uint16_t value = 0;

        if (stepDir > 0) {
          int target_val = lightComposite->getTargetValue();
          value = profile_val * (target_val - source_val) / DUTY_MAX + source_val;
        } else {
          value = profile_val * source_val / DUTY_MAX;
        }
        lightComposite->setCurrentValue(value);

        ledWrite(LED_COLOR(4), value);
        retVal = true;
      }
      if (shouldMoveOn() == false) {
        log_d("level: %u, stepDir: %d, currentValue: %u",level, stepDir, lightComposite->getCurrentValue());
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

void LedStripMgr::setDirAndLightComposite(int dir, LightComposite &composite) {
  portENTER_CRITICAL(&mux);
  stepDir = dir;
  if (lightComposite != &composite) {
    auto value = lightComposite->getCurrentValue();
    composite.setSourceValue(value);
    log_d("level: %u, stepDir: %d, sourceValue: %u",level, stepDir, value);

    lightComposite = &composite;
    lightComposite->setCurrentValue(value);
    level       = (stepDir > 0)? 0: composite.lastSampleNum();
    log_d("level: %u",level);
  } else {
    auto value = (stepDir > 0)? 0 : composite.getTargetValue();
    composite.setSourceValue(value);
  }
  portEXIT_CRITICAL(&mux);
}
