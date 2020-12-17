#include <Esp.h>+
#include "LedStripMgr.h"

const unsigned ALARM_LIGHTENING_PERIOD_MS = 4000;//20 * 60 * 1000;
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
  retVal = lightComposite->canMoveOn(stepDir);
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
  retVal = lightComposite->getLevel();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

Color_t LedStripMgr::getMaxValue() {
  Color_t retVal;
  portENTER_CRITICAL(&mux);
  retVal = lightComposite->getMaxValue();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

void LedStripMgr::setMaxValue(Color_t value) {
  portENTER_CRITICAL(&mux);
  alarmLightComposite.setMaxValue(value);
  switchLightComposite.setMaxValue(value);
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

void LedStripMgr::beginSettingMaxValue() {
  portENTER_CRITICAL(&mux);
  stepDir = 1;
  lightComposite = &switchLightComposite;
  lightComposite->setLevel(0);
  lightComposite->resetMaxValue();
  lightComposite->setSourceValue(Color_t {});
  lightComposite->setTargetValueToMaxValue();
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::finishSettingMaxValue() {
  portENTER_CRITICAL(&mux);
  stepDir = 0;
  // TODO: When setting max values is separated, this should be removed
  Color_t value = lightComposite->getCurrentValue();
  setMaxValue(value);
  lightComposite->setTargetValueToMaxValue();
  lightComposite->setLevelToMax();
  portEXIT_CRITICAL(&mux);
}

/*int cnt = 0;
void LedStripMgr::handleSwitch() {
  portENTER_CRITICAL(&mux);
  int newDir = (stepDir != 0)? -stepDir : (lightComposite->getLevel() == 0)? 1 : -1;
  if (cnt++ % 2 == 0) {
    setDirAndLightComposite(newDir, switchLightComposite);
  } else {
    setDirAndLightComposite(newDir, alarmLightComposite);
  }
  portEXIT_CRITICAL(&mux);
}*/

void LedStripMgr::handleSwitch() {
  portENTER_CRITICAL(&mux);
  int newDir = (stepDir != 0)? -stepDir : (lightComposite->getLevel() == 0)? 1 : -1;
  setDirAndLightComposite(newDir, switchLightComposite);
  portEXIT_CRITICAL(&mux);
}

bool LedStripMgr::changeLight(unsigned long timeSinceLastLightChange) {
  bool retVal = false;
  portENTER_CRITICAL(&mux);

  if (timeSinceLastLightChange > lightComposite->getSampleDuration()) {
    if (stepDir != 0) {
      if (shouldMoveOn()) {
        lightComposite->moveOn(stepDir);
        auto value = lightComposite->getCurrentValue();
        for (int i = 0; i < LED_LAST; i++) {
          ledWrite((LED_COLOR)i, value[i]);
        }
        retVal = true;
      }
      if (shouldMoveOn() == false) {
        log_d("stepDir: %d, level: %u, currentValue: %u", stepDir, lightComposite->getLevel(), lightComposite->getCurrentValue());
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
  ledcWrite(LEDC_CHANNEL_0 + color, val);
}

void LedStripMgr::setDirAndLightComposite(int dir, LightComposite &composite) {
  portENTER_CRITICAL(&mux);
  stepDir = dir;
  log_d("New Dir: %d", stepDir);
  if (lightComposite != &composite) {
    auto value = lightComposite->getCurrentValue();
    log_d("Old level: %u",lightComposite->getLevel());

    lightComposite = &composite;

    if (stepDir > 0) {
      lightComposite->setLevel(0);
      lightComposite->setSourceValue(value);
      lightComposite->setTargetValueToMaxValue();
    } else {
      lightComposite->setLevelToMax();
      lightComposite->setSourceValue(Color_t {});
      lightComposite->setTargetValue(value);
    }
    log_d("New level: %u, sourceValue: %u, targetValue: %u", lightComposite->getLevel(), lightComposite->getSourceValue(), lightComposite->getTargetValue());
  }
  portEXIT_CRITICAL(&mux);
}
