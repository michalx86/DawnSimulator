#include <assert.h>
#include "LightComposite.h"

LightComposite::LightComposite(const LightProfile& _profile, unsigned _period): profile(_profile), period(_period) {
  resetTargetValue();
}

unsigned LightComposite::samplesNum() const {
  return profile.samplesNum();
}

unsigned LightComposite::lastSampleNum() const {
  return profile.lastSampleNum();
}

uint16_t LightComposite::operator[](size_t idx) const {
  return profile[idx];
}

unsigned LightComposite::getSampleDuration() const {
  return period / samplesNum();
}

int LightComposite::getPercent() const {
  return currentValue * 100 / DUTY_MAX;
}

uint16_t LightComposite::getSourceValue() {
  return sourceValue;
}

void LightComposite::setSourceValue(uint16_t value) {
  sourceValue = value;
}

uint16_t LightComposite::getCurrentValue() {
  return currentValue;
}

void LightComposite::setCurrentValue(uint16_t value) {
  currentValue = value;
}

uint16_t LightComposite::getTargetValue() {
  return targetValue;
}

void LightComposite::setTargetValue(uint16_t value) {
  targetValue = value;
}

void LightComposite::resetTargetValue() {
  targetValue = DUTY_MAX;
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

uint16_t LightComposite::moveOn(int delta) {
  if (canMoveOn(delta)) {
    level += delta;
    int profile_val = (*this)[level];
    int source_val = this->getSourceValue();
    uint16_t value = 0;

    if (delta > 0) {
      int target_val = this->getTargetValue();
      value = profile_val * (target_val - source_val) / DUTY_MAX + source_val;
    } else {
      value = profile_val * source_val / DUTY_MAX;
    }
    this->setCurrentValue(value);
  }
  return this->getCurrentValue();
}
