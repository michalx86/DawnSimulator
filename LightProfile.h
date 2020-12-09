#ifndef LightProfile_h
#define LightProfile_h

#include <stddef.h>
#include <stdint.h>

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// calculate duty, 8191 from 2 ^ 13 - 1
const unsigned DUTY_MAX = (1 << LEDC_TIMER_13_BIT) - 1;

enum class LightProfileName {
  Alarm,
  Switch
};

class LightProfile {
public:
  LightProfile(LightProfileName profileName);
  unsigned samplesNum() const;
  unsigned lastSampleNum() const;
  uint16_t operator[](size_t idx) const;
private:
  LightProfileName profile;
};

#endif // LightProfile_h
