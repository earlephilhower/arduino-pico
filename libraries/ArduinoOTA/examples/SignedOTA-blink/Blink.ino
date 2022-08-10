// To be used with SignedOTA.  The IDE will sign the binary
// automatically and upload over WiFi

// Released to the public domain, Earle Philhower, 2022

#include <Arduino.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  srand(123);
}

void loop() {
  int del = rand() % 100;
  digitalWrite(LED_BUILTIN, HIGH);
  delay(del * 10);
  digitalWrite(LED_BUILTIN, LOW);
  delay(del * 10);
}
