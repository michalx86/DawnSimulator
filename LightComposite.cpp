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
