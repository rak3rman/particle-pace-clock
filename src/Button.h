#ifndef __BUTTON_H
#define __BUTTON_H

#include "Particle.h"

/**
 * @brief A debounced button/switch input handler class
 *
 * This class manages a digital input pin connected to a button or switch,
 * providing debounced state reading and change detection. It supports both
 * momentary buttons and toggle switches.
 */
class Button {
 public:
  /**
   * @brief Constructs a new Button instance
   * @param pin The digital input pin number to use
   *
   * Initializes the pin as INPUT_PULLDOWN, meaning the pin will read LOW when
   * the button is not pressed, and HIGH when pressed.
   */
  Button(int pin) : pin(pin) {
    pinMode(pin, INPUT_PULLDOWN);
  }

  /**
   * @brief Updates the button state with debouncing
   * @param now Current system time in milliseconds
   *
   * Should be called regularly (typically in loop()) to update the button
   * state. Implements debouncing to filter out noise and switch bounce, only
   * updating the actual state after the input has been stable for
   * DEBOUNCE_DELAY ms.
   */
  void update(system_tick_t now) {
    bool currentlyPressed = digitalRead(pin);

    if (currentlyPressed != lastReading) {
      lastDebounceTime = now;
    }

    if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
      // If the button state has changed after debounce:
      if (currentlyPressed != switchState) {
        switchState = currentlyPressed;
        // Set stateChanged flag when switch changes state
        stateChanged = true;
      }
    }

    lastReading = currentlyPressed;
  }

  /**
   * @brief Checks if the button/switch is currently in the ON state
   * @return true if the button is pressed/switch is on, false otherwise
   */
  bool isOn() const {
    return switchState;
  }

  /**
   * @brief Checks if the button state has changed since last check
   * @return true if state changed, false otherwise
   *
   * This is a "consume" operation - calling it clears the change flag,
   * so subsequent calls will return false until the state changes again.
   */
  bool stateJustChanged() {
    bool changed = stateChanged;
    stateChanged = false;
    return changed;
  }

 private:
  static const system_tick_t DEBOUNCE_DELAY = 50;  // 50ms debounce time

  const int pin;             // Digital input pin number
  bool lastReading = false;  // Last raw reading from the pin
  bool switchState = false;  // Current debounced state (on/off)
  bool stateChanged =
      false;  // Set when state changes, cleared by stateJustChanged()
  system_tick_t lastDebounceTime =
      0;  // Time of last state change for debouncing
};

#endif /* __BUTTON_H */