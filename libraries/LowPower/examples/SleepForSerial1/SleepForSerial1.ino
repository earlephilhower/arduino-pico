// Light sleep for 5 seconds in a loop, reporting over Serial1 (GP0 TX / GP1 RX on a Pico).
#include <LowPower.h>

void setup() {
  Serial1.begin(115200);
  delay(100);
}

void loop() {
  Serial1.printf("before sleep: millis=%lu\n", (unsigned long)millis());
  Serial1.flush(); // let the UART drain; its clock is gated while sleeping
  int rc = LowPower.sleepFor(5000);
  Serial1.printf("after sleep: rc=%d millis=%lu lastSleepMs=%lu\n", rc, (unsigned long)millis(),
                 (unsigned long)LowPower.lastSleepMs());
  delay(500);
}
