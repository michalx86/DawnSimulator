#include <assert.h>
#include <algorithm>
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
    int32_t target = targetValue.getComponent(compIdx);
    int32_t source = sourceValue.getComponent(compIdx);
    unsigned lev = level;
    if (source > target) {
      lev = lastSampleNum() - level;
      std::swap(target,source);
    }
    const int32_t profile_val = (*this)(compIdx,lev);
    retColor[compIdx] = profile_val * (target - source) / DUTY_MAX + source;
    assert(retColor[compIdx] <= DUTY_MAX);
  }
  return retColor;
}

Color_t LightComposite::getTargetValue() const {
  return targetValue;
}

void LightComposite::setTargetValue(Color_t value) {
  targetValue = value;
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
