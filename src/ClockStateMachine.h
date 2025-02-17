#ifndef __CLOCKSTATEMACHINE_H
#define __CLOCKSTATEMACHINE_H

#include "Button.h"
#include "Particle.h"
#include "SegmentDisplay.h"

/**
 * @brief Main control class for the pace clock
 *
 * Implements a state machine that manages different modes of operation:
 * - Sleep: Display off when power switch is off
 * - Manual Rainbow: Cycling rainbow colors display of elapsed time
 * - Manual Red: Red color display of elapsed time
 * - Countdown 50: Special countdown mode for swim training
 *
 * Handles button inputs, display updates, and mesh network synchronization
 * between multiple clocks.
 */
class ClockStateMachine {
 public:
  static ClockStateMachine* instance;

  ClockStateMachine();
  virtual ~ClockStateMachine();

  void setup();
  void loop();

  void recvMeshTime(const char* data);
  void pushTimeToMesh(
      int d1, int d2, int d3, int d4, int dot, int r, int g, int b);

 private:
  const int refreshInterval = 500;

  // Pin definitions
  static const int PIN_POWER = D7;
  static const int PIN_MANUAL_RAINBOW = D4;
  static const int PIN_MANUAL_RED = D5;
  static const int PIN_COUNTDOWN_50 = D6;

  // Hardware components
  Button powerSwitch;
  Button manualRainbowSwitch;
  Button manualRedSwitch;
  Button countdown50Switch;
  Adafruit_NeoPixel strip;
  SegmentDisplay display;

  // State handlers
  static void stateSleep(ClockStateMachine& csm);
  static void stateManualRainbow(ClockStateMachine& csm);
  static void stateManualRed(ClockStateMachine& csm);
  static void stateCountdown50(ClockStateMachine& csm);

  // State management
  void (*stateHandler)(ClockStateMachine&) = nullptr;
  system_tick_t startTime = 0;

  // Helper methods
  void resetTime();
  bool showInitialCountdown(uint32_t elapsedMs);
  void updateTimeFromMillis(int r, int g, int b);
  void updateTimeDisplay(
      int minutes, int seconds, int r, int g, int b, int dotStatus = 1);
  void calculateRainbowColor(uint8_t pos, int& r, int& g, int& b);

  // Input handling
  void updateButtons();

  // Display data encoding/decoding
  void encodeDisplayData(char* buffer,
                         int d1,
                         int d2,
                         int d3,
                         int d4,
                         int dot,
                         int r,
                         int g,
                         int b);
  bool decodeDisplayData(const char* data,
                         int& d1,
                         int& d2,
                         int& d3,
                         int& d4,
                         int& dot,
                         int& r,
                         int& g,
                         int& b);
};

#endif /* __CLOCKSTATEMACHINE_H */