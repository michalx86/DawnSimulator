#ifndef LightState_h
#define LightState_h

#include <stddef.h>
#include <stdint.h>
#include "LightProfile.h"

class LightState {
public:
  LightState(const LightProfile& _profile);
  unsigned samplesNum() const;
  uint16_t operator[](size_t idx) const;
  unsigned getSampleDuration() const;
  int toPercent(unsigned sample) const;
  unsigned sampleHigherOrEqual(unsigned value) const;
  unsigned getTargetLevel() const;
  void setTargetLevel(unsigned level);
  uint16_t getTargetLevelValue();
  void setTargetLevelFromValue(uint16_t value);
  void resetTargetLevel();
private:
  unsigned targetLevel = 0;
  const LightProfile& profile;
};

#endif // LightState_h
