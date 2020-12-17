#ifndef LightComposite_h
#define LightComposite_h

#include "LightProfile.h"

enum LED_COLOR {
  LED_R = 0, LED_G, LED_B, LED_WW, LED_CW, LED_LAST
};

typedef struct {
  uint16_t& operator[](size_t idx) {assert(idx < LED_LAST); return component[idx];}
  uint16_t getComponent(size_t idx) const {assert(idx < LED_LAST); return component[idx];}
  uint16_t component[LED_LAST] = {0,};
  int getPercent() const;
} Color_t;


class LightComposite {
public:
  LightComposite(const LightProfile& _profile, unsigned _period);
  unsigned lastSampleNum() const;
  unsigned getSampleDuration() const;
  int getPercent() const;
  Color_t getSourceValue()const;
  void setSourceValue(Color_t value);
  Color_t getCurrentValue() const;
  Color_t getTargetValue() const;
  void setTargetValue(Color_t value);
  void setTargetValueToMaxValue();
  Color_t getMaxValue() const;
  void setMaxValue(Color_t value);
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
  Color_t sourceValue;
  Color_t targetValue;
  Color_t maxValue;
  unsigned period = 0;
  unsigned level = 0;
};

#endif // LightComposite_h
