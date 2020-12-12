#include "LightComponent.h"

LightComponent::LightComponent(const LightProfile& _profile, unsigned _period): profile(_profile), period(_period) {
  resetTargetValue();
}

unsigned LightComponent::samplesNum() const {
  return profile.samplesNum();
}

unsigned LightComponent::lastSampleNum() const {
  return profile.lastSampleNum();
}

uint16_t LightComponent::operator[](size_t idx) const {
  return profile[idx];
}

unsigned LightComponent::getSampleDuration() const {
  return period / samplesNum();
}

int LightComponent::getPercent() const {
  return currentValue * 100 / DUTY_MAX;
}

uint16_t LightComponent::getSourceValue() {
  return sourceValue;
}

void LightComponent::setSourceValue(uint16_t value) {
  sourceValue = value;
}

uint16_t LightComponent::getCurrentValue() {
  return currentValue;
}

void LightComponent::setCurrentValue(uint16_t value) {
  currentValue = value;
}

uint16_t LightComponent::getTargetValue() {
  return targetValue;
}

void LightComponent::setTargetValue(uint16_t value) {
  targetValue = value;
}

void LightComponent::resetTargetValue() {
  targetValue = DUTY_MAX;
}
