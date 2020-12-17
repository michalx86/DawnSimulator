#include <assert.h>
#include "LightComposite.h"

int Color_t::getPercent() const {
  long sum = 0;
  for (int i = 0; i < LED_LAST; i++) {
    sum += component[i];
  }
  return sum * 100 / (DUTY_MAX * LED_LAST);
}


LightComposite::LightComposite(const LightProfile& _profile, unsigned _period): profile(_profile), period(_period) {
  resetMaxValue();
  setTargetValueToMaxValue();
}

unsigned LightComposite::lastSampleNum() const {
  return profile.lastSampleNum();
}

unsigned LightComposite::getSampleDuration() const {
  return period / samplesNum();
}

int LightComposite::getPercent() const {
  return  getCurrentValue().getPercent();
}

Color_t LightComposite::getSourceValue() const {
  return sourceValue;
}

void LightComposite::setSourceValue(Color_t value) {
  sourceValue = value;
}

Color_t LightComposite::getCurrentValue() const {
  Color_t retColor;
  for (int i = 0; i < LED_LAST; i++) {
    const int profile_val = (*this)[level];
    retColor[i] = profile_val * (targetValue.getComponent(i) - sourceValue.getComponent(i)) / DUTY_MAX + sourceValue.getComponent(i);
  }
  return retColor;
}

Color_t LightComposite::getTargetValue() const {
  return targetValue;
}

void LightComposite::setTargetValue(Color_t value) {
  targetValue = value;
}

void LightComposite::setTargetValueToMaxValue() {
  targetValue = maxValue;
}

Color_t LightComposite::getMaxValue() const {
  return maxValue;
}

void LightComposite::setMaxValue(Color_t value) {
  maxValue = value;
}

void LightComposite::resetMaxValue() {
  for (int i = 0; i < LED_LAST; i++) {
    maxValue[i] = DUTY_MAX;
  }
}

bool LightComposite::canMoveOn(int delta) {
  return (((delta > 0) && (level + delta <= lastSampleNum())) ||
          ((delta < 0) && (level >= -delta)));
}

unsigned LightComposite::getLevel() {
  return level;
}

void LightComposite::setLevel(unsigned _level) {
  assert(level < samplesNum());
  level = _level;
}

void LightComposite::setLevelToMax() {
  level = lastSampleNum();
}

void LightComposite::moveOn(int delta) {
  if (canMoveOn(delta)) {
    level += delta;
  }
}

unsigned LightComposite::samplesNum() const {
  return profile.samplesNum();
}

uint16_t LightComposite::operator[](size_t idx) const {
  return profile[idx];
}
