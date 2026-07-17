/* Earle F. Philhower, III <earlephilhower@yahoo.com> */
/* Released to the public domain */

#include <MouseBLE.h>

void setup() {
  Serial.begin(115200);
  MouseBLE.setAbsolute(true);
  MouseBLE.begin("CircleBLE Mouse");
  delay(5000);
  Serial.printf("Press BOOTSEL to move the mouse in a circle\n");
}

void loop() {
  if (BOOTSEL) {
    Serial.println("BARREL ROLLS!!!");
    float r = 1000;
    for (float cx = 1000; cx <= 16000; cx += 4000) {
      for (float cy = 1000; cy <= 16000; cy += 4000) {
        for (float a = 0.0f; a < 2.0f * 3.14159f; a += 0.1f) {
          float ax = r * cos(a);
          float ay = r * sin(a);
          MouseBLE.move(ax + cx, ay + cy, 0);
          delay(10);
        }
      }
    }
    MouseBLE.setBattery(random(0, 101)); // Set between 0...100%
    while (BOOTSEL) {
      delay(1);
    }
  }
}
