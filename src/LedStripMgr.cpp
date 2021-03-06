#include <Esp.h>
#include "LedStripMgr.h"

const unsigned ALARM_LIGHTENING_PERIOD_MS = 20 * 60 * 1000;
const unsigned SWITCH_LIGHTENING_PERIOD_MS = 2000;
const unsigned TRANSITION_LIGHTENING_PERIOD_MS = 2000;

// use 500 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     500



// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

const LightProfile alarmLightProfile(LightProfileName::Alarm);
const LightProfile switchLightProfile(LightProfileName::Switch);
const LightProfile transitionLightProfile(LightProfileName::Transition);
LightComposite alarmLightComposite(alarmLightProfile, ALARM_LIGHTENING_PERIOD_MS);
LightComposite switchLightComposite(switchLightProfile, SWITCH_LIGHTENING_PERIOD_MS);
LightComposite transitionLightComposite(transitionLightProfile, TRANSITION_LIGHTENING_PERIOD_MS);

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

Color_t LedStripMgr::getTargetValue() {
  Color_t retVal;
  portENTER_CRITICAL(&mux);
  retVal = lightComposite->getTargetValue();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

int LedStripMgr::getPercent() {
  int retVal = 0;
  portENTER_CRITICAL(&mux);
  retVal = lightComposite->getPercent();
  portEXIT_CRITICAL(&mux);
  return retVal;
}

LightComposite* LedStripMgr::profileName2Composite(LightProfileName profileName) {
  LightComposite* composite = NULL;
  switch (profileName) {
    case LightProfileName::Alarm  : composite = &alarmLightComposite; break;
    case LightProfileName::Switch : composite = &switchLightComposite; break;
    case LightProfileName::Transition : composite = &transitionLightComposite; break;
    default : break;
  }
  return composite;
}

void LedStripMgr::handleLightOn(LightProfileName profileName, Color_t toColor) {
  portENTER_CRITICAL(&mux);
  LightComposite* composite = profileName2Composite(profileName);

  stepDir = 1;
  if ((lightComposite != composite) || (lightComposite->getTargetValue() != toColor)) {
    auto value = lightComposite->getCurrentValue();
    auto oldLevel = lightComposite->getLevel();

    lightComposite = composite;

    lightComposite->setLevel(0);
    lightComposite->setSourceValue(value);
    lightComposite->setTargetValue(toColor);

    Color_t srcVal = lightComposite->getSourceValue();
    Color_t trgVal = lightComposite->getTargetValue();
    log_d("Level: %u -> %u, color: [%u,%u,%u,%u,%u] -> [%u,%u,%u,%u,%u]", oldLevel, lightComposite->getLevel(), srcVal[0], srcVal[1], srcVal[2], srcVal[3], srcVal[4], trgVal[0], trgVal[1], trgVal[2], trgVal[3], trgVal[4]);
  }
  portEXIT_CRITICAL(&mux);
}

void LedStripMgr::handleLightOff(LightProfileName profileName) {
  portENTER_CRITICAL(&mux);
  LightComposite* composite = profileName2Composite(profileName);

  stepDir = -1;
  if ((lightComposite != composite) || lightComposite->getSourceValue() != (Color_t {})) {
    auto value = lightComposite->getCurrentValue();
    auto oldLevel = lightComposite->getLevel();

    lightComposite = composite;

    lightComposite->setLevelToMax();
    lightComposite->setSourceValue(Color_t {});
    lightComposite->setTargetValue(value);

    Color_t srcVal = lightComposite->getSourceValue();
    Color_t trgVal = lightComposite->getTargetValue();
    log_d("Level: %u -> %u, color: [%u,%u,%u,%u,%u] -> [%u,%u,%u,%u,%u]", oldLevel, lightComposite->getLevel(), srcVal[0], srcVal[1], srcVal[2], srcVal[3], srcVal[4], trgVal[0], trgVal[1], trgVal[2], trgVal[3], trgVal[4]);
  }
  portEXIT_CRITICAL(&mux);
}

bool LedStripMgr::changeLight(unsigned long timeSinceLastLightChange) {
  bool retVal = false;
  portENTER_CRITICAL(&mux);

  if (timeSinceLastLightChange > lightComposite->getSampleDuration()) {
    if (stepDir != 0) {
      if (shouldMoveOn()) {
        lightComposite->moveOn(stepDir);
        auto currVal = lightComposite->getCurrentValue();
        //log_d("currentValue: [%u,%u,%u,%u,%u]", currVal[0], currVal[1],currVal[2],currVal[3],currVal[4]);

        for (int i = 0; i < LED_LAST; i++) {
          ledWrite((LED_COLOR)i, currVal[i]);
        }
        retVal = true;
      }
      if (shouldMoveOn() == false) {
        Color_t currVal = lightComposite->getCurrentValue();
        log_d("Transition Finished. stepDir: %d, level: %u, currentValue: [%u,%u,%u,%u,%u]", stepDir, lightComposite->getLevel(), currVal[0], currVal[1],currVal[2],currVal[3],currVal[4]);
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
  if (val > DUTY_MAX) {
    log_e("Component [%u] out of range: %u", color, val);
  }
  ledcWrite(LEDC_CHANNEL_0 + color, val);
}

void LedStripMgr::diagnostic() {
//  log_d("Alarm light profile:");
//  for (int i = 0; i <= 100; i++) { // alarmLightComposite.lastSampleNum()
//    log_d("%4d:  [%u,%u,%u,%u,%u]", i, alarmLightComposite(0,i), alarmLightComposite(1,i), alarmLightComposite(2,i), alarmLightComposite(3,i), alarmLightComposite(4,i));
//  }
//  log_d("Switch light profile:");
//  for (int i = 0; i <= switchLightComposite.lastSampleNum(); i++) {
//    log_d("%4d:  [%u,%u,%u,%u,%u]", i, switchLightComposite(0,i), switchLightComposite(1,i), switchLightComposite(2,i), switchLightComposite(3,i), switchLightComposite(4,i));
//  }
}
