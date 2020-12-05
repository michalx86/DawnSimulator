#include "LightState.h"

LightState::LightState(const LightProfile& _profile): profile(_profile) {
  resetTargetLevel();
}

unsigned LightState::samplesNum() const {
  return profile.samplesNum();
}

uint16_t LightState::operator[](size_t idx) const {
  return profile[idx];
}

unsigned LightState::getSampleDuration() const {
  return profile.getSampleDuration();
}

int LightState::toPercent(unsigned level) const {
  return profile.toPercent(level);
}

unsigned LightState::sampleHigherOrEqual(unsigned value) const {
  return profile.sampleHigherOrEqual(value);
}

unsigned LightState::getTargetLevel() const {
  return targetLevel;
}

void LightState::setTargetLevel(unsigned level) {
  targetLevel = level;
}

uint16_t LightState::getTargetLevelValue() {
  return profile[targetLevel];
}

void LightState::setTargetLevelFromValue(uint16_t value) {
  setTargetLevel(sampleHigherOrEqual(value));
}

void LightState::resetTargetLevel() {
  targetLevel = samplesNum() - 1;
}
