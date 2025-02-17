#ifndef __SEGMENTDISPLAY_H
#define __SEGMENTDISPLAY_H

#include <neopixel.h>

#include "Particle.h"

/**
 * @brief Controls a 4-digit seven-segment display made of NeoPixels
 *
 * Manages a strip of WS2812B LEDs arranged as four 7-segment digits plus
 * separator dots. Handles:
 * - Converting decimal digits to segment patterns
 * - Managing LED colors and brightness
 * - Different dot display patterns
 * - Loading/startup animations
 *
 * The display is arranged as:
 * [D1] [D2] [dots] [D3] [D4]
 * where each digit consists of 7 segments of multiple LEDs each.
 */
class SegmentDisplay {
 public:
  SegmentDisplay(Adafruit_NeoPixel& led_strip) : strip(led_strip) {}

  void setTime(int d1, int d2, int d3, int d4, int dot, int r, int g, int b);
  void loading();

 private:
  Adafruit_NeoPixel& strip;

  void updateDigit(uint8_t position, int8_t value);
  void updateDots(uint8_t mode);

  int curr_r, curr_g, curr_b;
};

#endif /* __SEGMENTDISPLAY_H */