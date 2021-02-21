#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// calculate duty, 8191 from 2 ^ 13 - 1
const unsigned DUTY_MAX = (1 << LEDC_TIMER_13_BIT) - 1;


enum LED_COLOR {
  LED_R = 0, LED_G, LED_B, LED_WW, LED_CW, LED_LAST
};

//struct Color_t;

struct Color_t {
  uint16_t& operator[](size_t idx) {assert(idx < LED_LAST); return component[idx];}
  uint16_t getComponent(size_t idx) const {assert(idx < LED_LAST); return component[idx];}
  int getPercent() const;
  bool operator!=(const Color_t &other_color) const {
    return memcmp(&component[0], &other_color.component[0], sizeof(component)) != 0;
  };
  uint16_t component[LED_LAST] = {0,};
};


#endif // UTILS_H