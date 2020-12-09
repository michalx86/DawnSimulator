#ifndef LightProfile_h
#define LightProfile_h

#include <stddef.h>
#include <stdint.h>

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 500 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     500


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
  unsigned getSampleDuration() const;
private:
  unsigned getPeriod() const;
  LightProfileName profile;
};

#endif // LightProfile_h
