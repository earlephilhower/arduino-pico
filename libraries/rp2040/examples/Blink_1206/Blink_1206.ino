#include "blink_config.h"
// This example is intended to be a minimal reproduction test case for earlephilhower/arduino-pico#1206

void setup() {
  pinMode(led, OUTPUT);
}

void loop() {
  digitalWrite(led, LOW);
  delay(blink_time);
  digitalWrite(led, HIGH);
  delay(blink_time);
}
