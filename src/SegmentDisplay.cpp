#include "SegmentDisplay.h"

static Logger log("segment_display");

/**
 * @brief Lookup table for 7-segment digit patterns
 *
 * Each row represents a digit (0-9) and contains 7 values for segments a-g.
 * 1 means the segment is on, 0 means off.
 * Segment layout:
 *   f
 * e   g
 *   a
 * d   b
 *   c
 */
const uint8_t DIGIT_PATTERNS[10][7] = {
    {0, 1, 1, 1, 1, 1, 1},  // 0
    {0, 1, 0, 0, 0, 0, 1},  // 1
    {1, 1, 1, 0, 1, 1, 0},  // 2
    {1, 1, 1, 0, 0, 1, 1},  // 3
    {1, 1, 0, 1, 0, 0, 1},  // 4
    {1, 0, 1, 1, 0, 1, 1},  // 5
    {1, 0, 1, 1, 1, 1, 1},  // 6
    {0, 1, 1, 0, 0, 0, 1},  // 7
    {1, 1, 1, 1, 1, 1, 1},  // 8
    {1, 1, 1, 1, 0, 0, 1}   // 9
};

/**
 * @brief LED mapping for each digit position
 *
 * Defines the LED ranges for each segment of each digit.
 * Each digit has 7 segments, and each segment consists of multiple LEDs.
 * segments[7][2] contains [start,end] LED indices for each segment.
 */
const struct {
  uint16_t start;
  uint16_t segments[7][2];  // [segment_index][start,end]
} DIGIT_POSITIONS[4] = {
    {0,
     {{0, 5},
      {6, 11},
      {12, 17},
      {18, 23},
      {24, 29},
      {30, 35},
      {36, 41}}},  // Digit 1
    {42,
     {{42, 47},
      {48, 53},
      {54, 59},
      {60, 65},
      {66, 71},
      {72, 77},
      {78, 83}}},  // Digit 2
    {92,
     {{92, 97},
      {98, 103},
      {104, 109},
      {110, 115},
      {116, 121},
      {122, 127},
      {128, 133}}},  // Digit 3
    {134,
     {{134, 139},
      {140, 145},
      {146, 151},
      {152, 157},
      {158, 163},
      {164, 169},
      {170, 175}}}  // Digit 4
};

/**
 * @brief Updates the display with new time and color values
 *
 * @param d1 First digit (leftmost)
 * @param d2 Second digit
 * @param d3 Third digit
 * @param d4 Fourth digit (rightmost)
 * @param dot Dot display mode (0=off, 1=on, other values for special patterns)
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void SegmentDisplay::setTime(
    int d1, int d2, int d3, int d4, int dot, int r, int g, int b) {
  log.info("SegmentDisplay::setTime()");

  // Store current state
  curr_r = r;
  curr_g = g;
  curr_b = b;

  // Update each digit
  updateDigit(0, d1);
  updateDigit(1, d2);
  updateDigit(2, d3);
  updateDigit(3, d4);

  // Update dots based on mode
  updateDots(dot);

  strip.show();
}

void SegmentDisplay::updateDigit(uint8_t position, int8_t value) {
  // Handle blank digit case
  const uint8_t* pattern = value == -1 ? nullptr : DIGIT_PATTERNS[value];

  // Update each segment for this digit
  for (int segment = 0; segment < 7; segment++) {
    bool isOn = pattern ? pattern[segment] : 0;
    uint16_t start = DIGIT_POSITIONS[position].segments[segment][0];
    uint16_t end = DIGIT_POSITIONS[position].segments[segment][1];

    for (uint16_t i = start; i <= end; i++) {
      strip.setPixelColor(i, isOn ? strip.Color(curr_r, curr_g, curr_b) : 0);
    }
  }
}

void SegmentDisplay::updateDots(uint8_t mode) {
  // Dot LED positions
  const uint16_t DOT_LEDS[] = {84, 85, 86, 87, 88, 89, 90, 91};
  const uint8_t DOT_COUNT = sizeof(DOT_LEDS) / sizeof(DOT_LEDS[0]);

  // Define dot patterns for different modes
  bool patterns[5][8] = {
      {1, 1, 0, 0, 0, 0, 0, 0},  // Mode 1, bottom 1/2 dot (ms)
      {0, 0, 0, 0, 0, 0, 0, 0},  // Mode 2, all off
      {1, 1, 0, 0, 0, 0, 1, 1},  // Mode 3, right 1/2 dot (sec)
      {0, 0, 1, 1, 1, 1, 0, 0},  // Mode 4, left 1/2 dot (sec)
      {1, 1, 1, 1, 1, 1, 1, 1}   // Mode 5, all on
  };

  // Validate mode
  if (mode < 1 || mode > 5)
    return;

  // Apply the pattern
  const bool* pattern = patterns[mode - 1];
  for (uint8_t i = 0; i < DOT_COUNT; i++) {
    strip.setPixelColor(DOT_LEDS[i],
                        pattern[i] ? strip.Color(curr_r, curr_g, curr_b) : 0);
  }
}

void SegmentDisplay::loading() {
  // Dark blue color
  curr_r = 0;
  curr_g = 0;
  curr_b = 64;

  // Light up sequence
  for (int digit = 0; digit < 4; digit++) {
    uint16_t start = DIGIT_POSITIONS[digit].segments[0][0];
    uint16_t end = DIGIT_POSITIONS[digit].segments[0][1];

    for (uint16_t i = start; i <= end; i++) {
      strip.setPixelColor(i, curr_r, curr_g, curr_b);
      strip.show();
      delay(15);
    }

    // Center dots
    if (digit == 1) {
      strip.setPixelColor(87, curr_r, curr_g, curr_b);
      strip.setPixelColor(88, curr_r, curr_g, curr_b);
      strip.show();
      delay(15);
    }
  }

  delay(100);  // Pause at full illumination

  // Turn off sequence
  for (int digit = 0; digit < 4; digit++) {
    uint16_t start = DIGIT_POSITIONS[digit].segments[0][0];
    uint16_t end = DIGIT_POSITIONS[digit].segments[0][1];

    for (uint16_t i = start; i <= end; i++) {
      strip.setPixelColor(i, 0, 0, 0);
      strip.show();
      delay(15);
    }

    // Center dots
    if (digit == 1) {
      strip.setPixelColor(87, 0, 0, 0);
      strip.setPixelColor(88, 0, 0, 0);
      strip.show();
      delay(15);
    }
  }

  strip.clear();
  strip.show();
}