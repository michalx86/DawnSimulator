#ifndef LightComposite_h
#define LightComposite_h

#include "LightComponent.h"

enum LED_COLOR {
  LED_R = 0, LED_G, LED_B, LED_WW, LED_CW, LED_LAST
};


class LightComposite {
public:
  LightComposite(const LightProfile& _profile, unsigned _period);
  unsigned lastSampleNum() const;
  unsigned getSampleDuration() const;
  int getPercent() const;
  uint16_t getSourceValue()const;
  void setSourceValue(uint16_t value);
  uint16_t getCurrentValue() const;
  uint16_t getTargetValue() const;
  void setTargetValue(uint16_t value);
  void setTargetValueToMaxValue();
  uint16_t getMaxValue() const;
  void setMaxValue(uint16_t value);
  void resetMaxValue();
  bool canMoveOn(int delta);
  unsigned getLevel();
  void setLevel(unsigned _level);
  void setLevelToMax();
  void moveOn(int delta);
private:
  unsigned samplesNum() const;
  uint16_t operator[](size_t component) const;

  const LightProfile& profile;
  uint16_t sourceValue = 0;
  uint16_t targetValue = 0;
  uint16_t maxValue = 0;
  unsigned period = 0;
  unsigned level = 0;
};

#endif // LightComposite_h
