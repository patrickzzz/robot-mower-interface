#include <Arduino.h>
#include "pins.h"

void setupPins() {
  // Start
  pinMode(pinButtonStart, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonStart, HIGH);

  // Home
  pinMode(pinButtonHome, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonHome, HIGH);

  // Stop
  pinMode(pinButtonStop, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonStop, LOW);

  // Locked
  pinMode(pinButtonLock, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonLock, HIGH);

  // Check LEDs
  pinMode(pinLedCharging, INPUT);
  pinMode(pinLedLocked, INPUT);
  pinMode(pinLedEmergency, INPUT);

  // Check Power on Pin5 (Idle)
  pinMode(pinIdle, INPUT);
}