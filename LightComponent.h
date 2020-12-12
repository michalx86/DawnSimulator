#ifndef LightComponent_h
#define LightComponent_h

#include "LightProfile.h"

class LightComponent {
public:
  LightComponent(const LightProfile& _profile, unsigned _period);
  unsigned samplesNum() const;
  unsigned lastSampleNum() const;
  uint16_t operator[](size_t idx) const;
  unsigned getSampleDuration() const;
  int getPercent() const;
  uint16_t getSourceValue();
  void setSourceValue(uint16_t value);
  uint16_t getCurrentValue();
  void setCurrentValue(uint16_t value);
  uint16_t getTargetValue();
  void setTargetValue(uint16_t value);
  void resetTargetValue();
private:
  const LightProfile& profile;
  uint16_t sourceValue = 0;
  uint16_t currentValue = 0;
  uint16_t targetValue = 0;
  unsigned period = 0;
};

#endif // LightComponent_h
