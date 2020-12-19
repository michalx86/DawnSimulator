#include <stddef.h>
#include <assert.h>
#include "LightProfile.h"
#include <Esp.h>

typedef struct {
  float low;
  float high;
} Boundaries_t;

static Boundaries_t alarm_boundaries[LED_LAST] = {{.0f, .35f}, {.0f, .55f}, {.25f, .75f}, {.25f, 1.0f}, {.4f, 1.0f}};
static Boundaries_t switch_boundaries[LED_LAST] = {{.0f, .35f}, {.0f, .55f}, {.25f, .75f}, {.25f, 1.0f}, {.4f, 1.0f}};

static uint16_t alarm_arr[LED_LAST][2000] = {};
static uint16_t switch_arr[LED_LAST][120] = {};

LightProfile::LightProfile(LightProfileName profileName): profile(profileName) {
  auto num_samples = samplesNum();
  for (int compIdx = 0; compIdx < LED_LAST; compIdx++) {
    uint16_t* arr = nullptr;
    float low = .0f;
    float high = .0f;
    switch (profile) {
      case LightProfileName::Alarm : arr = &alarm_arr[compIdx][0]; low = alarm_boundaries[compIdx].low; high = alarm_boundaries[compIdx].high; break;
      case LightProfileName::Switch : arr = &switch_arr[compIdx][0]; low = switch_boundaries[compIdx].low; high = switch_boundaries[compIdx].high; break;
      default : break;
    }
    assert(arr);
    assert((low >= .0f) && (low <= 1.0f));
    assert((high >= .0f) && (high <= 1.0f));
    assert(low <= high);
    unsigned lowIdx = low * num_samples;
    unsigned highIdx = high * num_samples;

    if (lowIdx == 0) {
      lowIdx = 1;
    }
    for (unsigned i = 0; i < lowIdx; i++) {
      arr[i] = 0;
    }

    unsigned long long num_varying_samples = highIdx - lowIdx;
    unsigned long long divisor = num_varying_samples * num_varying_samples;
    unsigned long long val = 1;
    for (unsigned i = lowIdx; i < highIdx; i++, val++) {
      arr[i] = (unsigned long long)DUTY_MAX * val * val / divisor;
    }

    for (unsigned long i = highIdx; i < num_samples; i++) {
      arr[i] = DUTY_MAX;
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
