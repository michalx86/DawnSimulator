#ifndef LightComposite_h
#define LightComposite_h

#include "LightComponent.h"

enum LED_COLOR {
  LED_R = 0, LED_G, LED_B, LED_WW, LED_CW, LED_LAST
};


class LightComposite {
public:
  LightComposite(const LightProfile& _profile, unsigned _period);
  unsigned samplesNum() const;
  unsigned lastSampleNum() const;
  uint16_t operator[](size_t component) const;
  unsigned getSampleDuration() const;
  int getPercent() const;
  uint16_t getSourceValue();
  void setSourceValue(uint16_t value);
  uint16_t getCurrentValue();
  void setCurrentValue(uint16_t value);
  uint16_t getTargetValue();
  void setTargetValue(uint16_t value);
  void resetTargetValue();
  bool canMoveOn(int delta);
  unsigned getLevel();
  void setLevel(unsigned _level);
  void setLevelToMax();
  uint16_t moveOn(int delta);
private:
  const LightProfile& profile;
  uint16_t sourceValue = 0;
  uint16_t currentValue = 0;
  uint16_t targetValue = 0;
  unsigned period = 0;
  unsigned level = 0;
};

#endif // LightComposite_h
