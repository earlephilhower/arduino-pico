/* Earle F. Philhower, III <earlephilhower@yahoo.com> */
/* Released to the public domain */

#include <MouseBT.h>

void setup() {
  Serial.begin(115200);
  MouseBT.setAbsolute(true);
  MouseBT.begin("CircleBT Mouse");
  delay(5000);
  Serial.printf("Press BOOTSEL to move the mouse in a circle\n");
}

void loop() {
  if (BOOTSEL) {
    Serial.println("BARREL ROLLS!!!");
    float r = 1000;
    for (float cx = 1000; cx <= 16000; cx += 4000) {
      for (float cy = 1000; cy <= 16000; cy += 4000) {
        for (float a = 0; a < 2.0 * 3.14159; a += 0.1) {
          float ax = r * cos(a);
          float ay = r * sin(a);
          MouseBT.move(ax + cx, ay + cy, 0);
          delay(10);
        }
      }
    }
    while (BOOTSEL) {
      delay(1);
    }
  }
}
