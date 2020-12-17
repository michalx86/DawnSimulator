#include <stddef.h>
#include <assert.h>
#include "LightProfile.h"


static uint16_t alarm_arr[LED_LAST][10] = {};
static uint16_t switch_arr[LED_LAST][120] = {};

LightProfile::LightProfile(LightProfileName profileName): profile(profileName) {
  auto num_samples = samplesNum();
  for (int compIdx = 0; compIdx < LED_LAST; compIdx++) {
    uint16_t* arr = nullptr;
    switch (profile) {
      case LightProfileName::Alarm : arr = &alarm_arr[compIdx][0]; break;
      case LightProfileName::Switch : arr = &switch_arr[compIdx][0]; break;
      default : break;
    }
    assert(arr);
    for (unsigned long i = 0; i < num_samples; i++) {
      arr[i] = (unsigned long)DUTY_MAX * i * i / (unsigned long)(num_samples-1) / (unsigned long)(num_samples-1);
    }
  }
}

unsigned LightProfile::samplesNum() const {
  switch (profile) {
    case LightProfileName::Alarm : return sizeof(alarm_arr[0]) / sizeof(alarm_arr[0][0]);
    case LightProfileName::Switch : return sizeof(switch_arr[0]) / sizeof(switch_arr[0][0]);
    default : break;
  }
  return 0;
}

unsigned LightProfile::lastSampleNum() const {
  return samplesNum() - 1;
}

uint16_t LightProfile::operator()(size_t compIdx, size_t idx) const {
  assert(compIdx < LED_LAST);
  assert(idx < samplesNum());
  switch (profile) {
    case LightProfileName::Alarm : return alarm_arr[compIdx][idx];
    case LightProfileName::Switch : return switch_arr[compIdx][idx];
    default : break;
  }
  return 0;
}
