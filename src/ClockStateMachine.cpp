#include "ClockStateMachine.h"

/**
 * @brief Singleton instance pointer for mesh network callback access
 */
ClockStateMachine* ClockStateMachine::instance = nullptr;

/**
 * @brief Constructor initializes hardware components and state
 *
 * Sets up:
 * - Button inputs with pulldown resistors
 * - NeoPixel LED strip (176 LEDs on pin D8)
 * - Initial time reset
 */
ClockStateMachine::ClockStateMachine()
    : powerSwitch(PIN_POWER),
      manualRainbowSwitch(PIN_MANUAL_RAINBOW),
      manualRedSwitch(PIN_MANUAL_RED),
      countdown50Switch(PIN_COUNTDOWN_50),
      strip(176, D8, WS2812B),
      display(strip) {
  resetTime();
}

ClockStateMachine::~ClockStateMachine() {}

/**
 * @brief Mesh network event handler callback
 *
 * Static function that routes mesh network events to the singleton instance.
 * Used as callback for Particle mesh network events.
 *
 * @param event Event name (unused)
 * @param data Display state data string
 */
static void meshTimeHandler(const char* event, const char* data) {
  if (ClockStateMachine::instance) {
    ClockStateMachine::instance->recvMeshTime(data);
  }
}

/**
 * @brief Initializes the clock hardware and network connection
 *
 * - Sets up NeoPixel strip
 * - Configures built-in RGB LED
 * - Initializes mesh network
 * - Runs startup animation while waiting for network
 */
void ClockStateMachine::setup() {
  // Set the static instance
  instance = this;

  // Initialize NeoPixel strip
  strip.begin();
  strip.clear();
  strip.show();

  // Initialize built-in RGB LED
  RGB.control(true);
  RGB.color(0, 0, 250);

  // Set initial state
  stateHandler = &stateSleep;

  // Configure mesh
  Mesh.on();
  Mesh.connect();
  Mesh.subscribe("meshTime", meshTimeHandler);

  // Run startup animation
  delay(500);
  while (!Mesh.ready()) {
    display.loading();
  }
}

/**
 * @brief Main update loop
 *
 * Updates button states and executes current state handler function
 */
void ClockStateMachine::loop() {
  updateButtons();

  // Execute current state
  if (stateHandler) {
    stateHandler(*this);
  }
}

/**
 * @brief Helper function for state update timing
 *
 * @param lastUpdate Last update timestamp
 * @param interval Desired update interval in milliseconds
 * @return true if enough time has elapsed since last update
 */
bool shouldUpdate(system_tick_t& lastUpdate, unsigned int interval) {
  system_tick_t now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;
    return true;
  }
  return false;
}

/**
 * @brief Sleep state handler - display off
 *
 * Display is turned off and RGB LED set to dim blue.
 * Transitions to active state when power switch is turned on.
 *
 * @param csm Reference to state machine instance
 */
void ClockStateMachine::stateSleep(ClockStateMachine& csm) {
  static bool firstEntry = true;

  if (firstEntry) {
    csm.pushTimeToMesh(-1, -1, -1, -1, 1, 0, 0, 0);
    RGB.color(0, 0, 10);
    firstEntry = false;
  }

  // Transitions
  if (csm.powerSwitch.isOn()) {
    // Power switch is on, transition to selected state
    if (csm.manualRedSwitch.isOn()) {
      csm.stateHandler = &stateManualRed;
    } else if (csm.countdown50Switch.isOn()) {
      csm.stateHandler = &stateCountdown50;
    } else {
      csm.stateHandler = &stateManualRainbow;
    }
    // Set initial state
    RGB.color(0, 10, 0);
    csm.resetTime();
    firstEntry = true;
  }
}

/**
 * @brief Rainbow color mode state handler
 *
 * Displays elapsed time with cycling rainbow colors.
 * Updates every refreshInterval milliseconds.
 *
 * @param csm Reference to state machine instance
 */
void ClockStateMachine::stateManualRainbow(ClockStateMachine& csm) {
  static system_tick_t lastUpdate = 0;

  if (shouldUpdate(lastUpdate, csm.refreshInterval)) {
    uint32_t elapsedSec = millis() / 1000;
    uint8_t wheelPos = (elapsedSec % 240) * 1.0625;  // 255/240 ≈ 1.0625

    int r, g, b;
    csm.calculateRainbowColor(wheelPos, r, g, b);
    csm.updateTimeFromMillis(r, g, b);
  }

  // Transitions
  if (!csm.powerSwitch.isOn()) {
    csm.stateHandler = &stateSleep;
    return;
  }
  if (csm.manualRedSwitch.isOn()) {
    csm.stateHandler = &stateManualRed;
  } else if (csm.countdown50Switch.isOn()) {
    csm.resetTime();
    csm.stateHandler = &stateCountdown50;
  }
}

/**
 * @brief Red color mode state handler
 *
 * Displays elapsed time in solid red color.
 * Updates every refreshInterval milliseconds.
 *
 * @param csm Reference to state machine instance
 */
void ClockStateMachine::stateManualRed(ClockStateMachine& csm) {
  static system_tick_t lastUpdate = 0;

  if (shouldUpdate(lastUpdate, csm.refreshInterval)) {
    csm.updateTimeFromMillis(255, 0, 0);
  }

  // Transitions
  if (!csm.powerSwitch.isOn()) {
    csm.stateHandler = &stateSleep;
    return;
  }
  if (csm.manualRainbowSwitch.isOn()) {
    csm.stateHandler = &stateManualRainbow;
  } else if (csm.countdown50Switch.isOn()) {
    csm.resetTime();
    csm.stateHandler = &stateCountdown50;
  }
}

/**
 * @brief 50-minute countdown state handler
 *
 * Implements a specialized countdown timer for swim training:
 * - Initial 22 second preparation period with two 10-second countdowns
 * - Main countdown starting at 60 seconds, decreasing by 1 second each round
 * - Color changes indicate time remaining in each countdown
 * - Group number shows current phase
 *
 * @param csm Reference to state machine instance
 */
void ClockStateMachine::stateCountdown50(ClockStateMachine& csm) {
  static system_tick_t lastUpdate = 0;

  if (shouldUpdate(lastUpdate, csm.refreshInterval)) {
    uint32_t now = millis();

    int8_t initialTime = 60;
    int32_t elapsedSec = ((now - csm.startTime) / 1000) - 30;
    int32_t totalSec = 0;

    // Calculate current round
    while (elapsedSec >= totalSec + initialTime) {
      totalSec += initialTime;
      initialTime--;
    }

    // Check if we're done
    if (initialTime <= 20) {
      csm.pushTimeToMesh(-1, -1, -1, -1, 1, 0, 0, 0);
      return;
    }

    uint32_t roundElapsedSec =
        elapsedSec < 0 ? initialTime + elapsedSec : elapsedSec - totalSec;
    uint8_t remainingSec;
    int r, g, b;
    int group = 0;

    // First 22 seconds: two 10-second countdowns
    if (roundElapsedSec < (initialTime < 30 ? 12 : 22)) {
      remainingSec = 10 - (roundElapsedSec % 10);

      // Set group based on phase
      group = (roundElapsedSec < 2) ? 1 : (roundElapsedSec < 12) ? 2 : 3;

      // Color selection
      if (remainingSec > 8) {
        r = 255;
        g = 0;
        b = 0;
        remainingSec = initialTime == 60 ? 0 : initialTime + 1;
      } else {
        r = (remainingSec > 6) ? 100 : (remainingSec > 3) ? 255 : 255;
        g = (remainingSec > 6) ? 255 : (remainingSec > 3) ? 255 : 100;
        b = 0;
      }
    }
    // Normal round timing
    else {
      remainingSec = initialTime - (roundElapsedSec % initialTime);
      group = (remainingSec <= 10) ? 1 : 0;

      // Color selection
      r = (remainingSec > 6) ? 100 : (remainingSec > 3) ? 255 : 255;
      g = (remainingSec > 6) ? 255 : (remainingSec > 3) ? 255 : 100;
      b = 0;
    }

    // Update display
    csm.pushTimeToMesh(group == 0 ? -1 : group, -1, remainingSec / 10,
                       remainingSec % 10, ((now % 1000) < 500) ? 3 : 4, r, g,
                       b);
  }

  // Transitions
  if (!csm.powerSwitch.isOn()) {
    csm.stateHandler = &stateSleep;
    return;
  }
  if (csm.manualRainbowSwitch.isOn()) {
    csm.resetTime();
    csm.stateHandler = &stateManualRainbow;
  } else if (csm.manualRedSwitch.isOn()) {
    csm.resetTime();
    csm.stateHandler = &stateManualRed;
  }
}

/**
 * @brief Resets the start time to current time
 *
 * Used when transitioning between states to restart timing
 */
void ClockStateMachine::resetTime() {
  startTime = millis();
}

/**
 * @brief Updates display with elapsed time in specified color
 *
 * Converts milliseconds since start into minutes:seconds format.
 * Handles special cases:
 * - Over 4 hours: display turns off
 * - Over 60 minutes: wraps around
 * - Leading zero suppression for minutes
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void ClockStateMachine::updateTimeFromMillis(int r, int g, int b) {
  uint32_t now = millis();

  // Normal time display
  uint32_t elapsedMs = now - startTime;
  uint32_t elapsedSec = elapsedMs / 1000;

  // Check if more than 4 hours have elapsed (4 * 60 * 60 = 14400 seconds)
  if (elapsedSec >= 14400) {
    r = g = b = 0;  // Turn off display
  }

  // Wrap around at 60 minutes (3600 seconds)
  elapsedSec = elapsedSec % 3600;
  uint8_t minutes = (elapsedSec / 60);
  uint8_t seconds = elapsedSec % 60;

  // Alternate dot status every 500ms
  int dotStatus = ((elapsedMs % 1000) < 500) ? 3 : 4;

  pushTimeToMesh(minutes / 10 == 0 ? -1 : minutes / 10, minutes % 10,
                 seconds / 10, seconds % 10, dotStatus, r, g, b);
}

/**
 * @brief Calculates RGB values for rainbow effect
 *
 * Converts position (0-255) into smooth RGB transitions:
 * 0-84: Red to Green
 * 85-169: Green to Blue
 * 170-255: Blue to Red
 *
 * @param pos Position in color wheel (0-255)
 * @param r Output red component
 * @param g Output green component
 * @param b Output blue component
 */
void ClockStateMachine::calculateRainbowColor(uint8_t pos,
                                              int& r,
                                              int& g,
                                              int& b) {
  if (pos < 85) {
    r = pos * 3;
    g = 255 - pos * 3;
    b = 0;
  } else if (pos < 170) {
    pos -= 85;
    r = 255 - pos * 3;
    g = 0;
    b = pos * 3;
  } else {
    pos -= 170;
    r = 0;
    g = pos * 3;
    b = 255 - pos * 3;
  }
}

/**
 * @brief Updates all button states
 *
 * Calls update() on all button inputs with current time
 */
void ClockStateMachine::updateButtons() {
  system_tick_t now = millis();
  powerSwitch.update(now);
  manualRainbowSwitch.update(now);
  manualRedSwitch.update(now);
  countdown50Switch.update(now);
}

/**
 * @brief Encodes display state into network message
 *
 * Format: "D1,D2,D3,D4,DOT,R,G,B"
 * Example: "-1,5,4,2,3,255,128,0"
 *
 * @param buffer Output buffer (min 32 bytes)
 * @param d1-d4 Digits to display (-1 for blank)
 * @param dot Dot display mode
 * @param r,g,b Color components
 */
void ClockStateMachine::encodeDisplayData(char* buffer,
                                          int d1,
                                          int d2,
                                          int d3,
                                          int d4,
                                          int dot,
                                          int r,
                                          int g,
                                          int b) {
  // Format: D1,D2,D3,D4,DOT,R,G,B
  // Example: "-1,5,4,2,3,255,128,0"
  snprintf(buffer, 32, "%d,%d,%d,%d,%d,%d,%d,%d", d1, d2, d3, d4, dot, r, g, b);
}

/**
 * @brief Sends current display state to other clocks via mesh network
 *
 * @param d1-d4 The four digits to display
 * @param dot Dot display mode
 * @param r,g,b Color components
 *
 * Encodes the display state into a string and broadcasts it to all
 * connected clocks in the mesh network.
 */
void ClockStateMachine::pushTimeToMesh(
    int d1, int d2, int d3, int d4, int dot, int r, int g, int b) {
  char encodedData[32];
  encodeDisplayData(encodedData, d1, d2, d3, d4, dot, r, g, b);
  recvMeshTime(encodedData);              // Update local display
  Mesh.publish("meshTime", encodedData);  // Broadcast to network
}

/**
 * @brief Decodes display data received from mesh network
 *
 * @param data Comma-separated string of display values
 * @param d1-d4 Output parameters for digits
 * @param dot Output parameter for dot mode
 * @param r,g,b Output parameters for color
 * @return true if parsing succeeded, false otherwise
 */
bool ClockStateMachine::decodeDisplayData(const char* data,
                                          int& d1,
                                          int& d2,
                                          int& d3,
                                          int& d4,
                                          int& dot,
                                          int& r,
                                          int& g,
                                          int& b) {
  return (sscanf(data, "%d,%d,%d,%d,%d,%d,%d,%d", &d1, &d2, &d3, &d4, &dot, &r,
                 &g, &b) == 8);
}

/**
 * @brief Handles incoming display data from mesh network
 *
 * @param data Encoded string containing display state
 *
 * Decodes the received data and updates the local display to match
 * the received state, keeping all clocks synchronized.
 */
void ClockStateMachine::recvMeshTime(const char* data) {
  int d1, d2, d3, d4, dot, r, g, b;
  if (decodeDisplayData(data, d1, d2, d3, d4, dot, r, g, b)) {
    display.setTime(d1, d2, d3, d4, dot, r, g, b);
  }
}