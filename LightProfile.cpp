#include <stddef.h>
#include <assert.h>
#include "LightProfile.h"


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

unsigned LightProfile::samplesNum() const {
  switch (profile) {
    case LightProfileName::Alarm : return sizeof(alarm_arr) / sizeof(alarm_arr[0]);
    case LightProfileName::Switch : return sizeof(switch_arr) / sizeof(switch_arr[0]);
    default : break;
  }
  return 0;
}

unsigned LightProfile::lastSampleNum() const {
  return samplesNum() - 1;
}

uint16_t LightProfile::operator[](size_t idx) const {
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
