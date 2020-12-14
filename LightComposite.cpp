#include <assert.h>
#include "LightComposite.h"

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
  return getCurrentValue() * 100 / DUTY_MAX;
}

uint16_t LightComposite::getSourceValue() const {
  return sourceValue;
}

void LightComposite::setSourceValue(uint16_t value) {
  sourceValue = value;
}

uint16_t LightComposite::getCurrentValue() const {
  int profile_val = (*this)[level];
  int source_val = this->getSourceValue();
  int target_val = this->getTargetValue();
  return profile_val * (target_val - source_val) / DUTY_MAX + source_val;
}

uint16_t LightComposite::getTargetValue() const {
  return targetValue;
}

void LightComposite::setTargetValue(uint16_t value) {
  targetValue = value;
}

void LightComposite::setTargetValueToMaxValue() {
  targetValue = maxValue;
}

uint16_t LightComposite::getMaxValue() const {
  return maxValue;
}

void LightComposite::setMaxValue(uint16_t value) {
  maxValue = value;
}

void LightComposite::resetMaxValue() {
  maxValue = DUTY_MAX;
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
