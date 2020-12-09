#include "LightState.h"

LightState::LightState(const LightProfile& _profile, unsigned _period): profile(_profile), period(_period) {
  resetTargetValue();
}

unsigned LightState::samplesNum() const {
  return profile.samplesNum();
}

unsigned LightState::lastSampleNum() const {
  return profile.lastSampleNum();
}

uint16_t LightState::operator[](size_t idx) const {
  return profile[idx];
}

unsigned LightState::getSampleDuration() const {
  return period / samplesNum();
}

int LightState::getPercent() const {
  return currentValue * 100 / DUTY_MAX;
}

uint16_t LightState::getSourceValue() {
  return sourceValue;
}

void LightState::setSourceValue(uint16_t value) {
  sourceValue = value;
}

uint16_t LightState::getCurrentValue() {
  return currentValue;
}

void LightState::setCurrentValue(uint16_t value) {
  currentValue = value;
}

uint16_t LightState::getTargetValue() {
  return targetValue;
}

void LightState::setTargetValue(uint16_t value) {
  targetValue = value;
}

void LightState::resetTargetValue() {
  targetValue = DUTY_MAX;
}
