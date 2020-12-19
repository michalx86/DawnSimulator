#include <assert.h>
#include "LightComposite.h"

int Color_t::getPercent() const {
  long sum = 0;
  for (int i = 0; i < 3; i++) {
    sum += component[i] / 2;
  }
  for (int i = 3; i < LED_LAST; i++) {
    sum += component[i];
  }
  unsigned half_duty = DUTY_MAX / 2;
  return sum * 100 / (3 * half_duty + 2* DUTY_MAX);
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
  for (int compIdx = 0; compIdx < LED_LAST; compIdx++) {
    const int profile_val = (*this)(compIdx,level);
    retColor[compIdx] = profile_val * (targetValue.getComponent(compIdx) - sourceValue.getComponent(compIdx)) / DUTY_MAX + sourceValue.getComponent(compIdx);
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
  setTargetValueToMaxValue();
}

void LightComposite::resetMaxValue() {
  Color_t color;
  for (int i = 0; i < LED_LAST; i++) {
    color[i] = DUTY_MAX;
  }
  setMaxValue(color);
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

uint16_t LightComposite::operator()(size_t compIdx, size_t idx) const {
  return profile(compIdx,idx);
}
