#include "ClockStateMachine.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

ClockStateMachine clockStateMachine;

void setup() {
  clockStateMachine.setup();
}

void loop() {
  clockStateMachine.loop();
}