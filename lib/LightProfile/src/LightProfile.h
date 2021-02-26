#ifndef LightProfile_h
#define LightProfile_h

#include <stddef.h>
#include <stdint.h>
#include "utils.h"

enum class LightProfileName {
  Alarm,
  Switch,
  Transition
};

class LightProfile {
public:
  LightProfile(LightProfileName profileName);
  unsigned samplesNum() const;
  unsigned lastSampleNum() const;
  uint16_t operator()(size_t compIdx, size_t idx) const;
private:
  LightProfileName profile;
};

#endif // LightProfile_h
