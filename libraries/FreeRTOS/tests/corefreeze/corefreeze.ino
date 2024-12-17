#include <Arduino.h>
#include <FreeRTOS.h>
#include "LittleFS.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  LittleFS.format();
}

void setup1() {
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
}

int x = 0;
void loop1() {
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  Serial.printf("%d\n", x++);
}
