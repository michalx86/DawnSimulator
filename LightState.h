#ifndef LightState_h
#define LightState_h

#include <stddef.h>
#include <stdint.h>
#include "LightProfile.h"

class LightState {
public:
  LightState(const LightProfile& _profile);
  unsigned samplesNum() const;
  unsigned lastSampleNum() const;
  uint16_t operator[](size_t idx) const;
  unsigned getSampleDuration() const;
  int toPercent(unsigned sample) const;
  uint16_t getSourceValue();
  void setSourceValue(uint16_t value);
  uint16_t getCurrentValue();
  void setCurrentValue(uint16_t value);
  uint16_t getTargetValue();
  void setTargetValue(uint16_t value);
  void resetTargetValue();
private:
  uint16_t sourceValue = 0;
  uint16_t currentValue = 0;
  uint16_t targetValue = 0;
  const LightProfile& profile;
};

#endif // LightState_h
