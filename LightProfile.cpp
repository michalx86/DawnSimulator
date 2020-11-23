#include <stddef.h>
#include <assert.h>
#include "LightProfile.h"


const unsigned ALARM_LIGHTENING_PERIOD_MS = 20 * 60 * 1000;
const unsigned SWITCH_LIGHTENING_PERIOD_MS = 2000;

static uint16_t alarm_arr[10] = {};
static uint16_t switch_arr[120] = {};

LightProfile::LightProfile(LightProfileName profileName): profile(profileName) {
  uint16_t* arr = nullptr;
  switch (profile) {
    case LightProfileName::Alarm : arr = &alarm_arr[0]; break;
    case LightProfileName::Switch : arr = &switch_arr[0]; break;
    default : break;
  }
  assert(arr);
  auto num_samples = samplesNum();
  for (unsigned long i = 0; i < num_samples; i++) {
    arr[i] = (unsigned long)DUTY_MAX * i * i / (unsigned long)(num_samples-1) / (unsigned long)(num_samples-1);
  }
}

unsigned LightProfile::samplesNum() {
  switch (profile) {
    case LightProfileName::Alarm : return sizeof(alarm_arr) / sizeof(alarm_arr[0]);
    case LightProfileName::Switch : return sizeof(switch_arr) / sizeof(switch_arr[0]);
    default : break;
  }
  return 0;
}

uint16_t LightProfile::operator[](size_t idx) {
  //assert(idx < samplesNum());
  if (idx >= samplesNum()) {
    idx = samplesNum();
  }
  switch (profile) {
    case LightProfileName::Alarm : return alarm_arr[idx];
    case LightProfileName::Switch : return switch_arr[idx];
    default : break;
  }
  return 0;
}

unsigned LightProfile::getPeriod() {
  switch (profile) {
    case LightProfileName::Alarm : return ALARM_LIGHTENING_PERIOD_MS;
    case LightProfileName::Switch : return SWITCH_LIGHTENING_PERIOD_MS;
    default : break;
  }
  return 0;
}

unsigned LightProfile::getSampleDuration() {
  return getPeriod() / samplesNum();
}

int LightProfile::toPercent(unsigned sample) {
    return (*this)[sample] * 100 / DUTY_MAX;
}

unsigned LightProfile::sampleHigherOrEqual(unsigned value) {
  for (int i = 0; i < samplesNum(); i++) {
    if ((*this)[i] >= value) {
      return i;
    }
  }
  return 0;
}
