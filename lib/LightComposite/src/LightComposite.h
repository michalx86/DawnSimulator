#ifndef LightComposite_h
#define LightComposite_h

#include "LightProfile.h"

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
  uint16_t operator()(size_t compIdx, size_t idx) const;
private:
  unsigned samplesNum() const;

  const LightProfile& profile;
  Color_t sourceValue;
  Color_t targetValue;
  Color_t maxValue;
  unsigned period = 0;
  unsigned level = 0;
};

#endif // LightComposite_h
