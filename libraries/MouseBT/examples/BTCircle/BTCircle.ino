/* Earle F. Philhower, III <earlephilhower@yahoo.com> */
/* Released to the public domain */


#include <MouseBT.h>

void setup() {
  Serial.begin(115200);
  MouseBT.begin("BarrelMouse");
  delay(5000);
  Serial.printf("Press BOOTSEL to move the mouse in a circle\n");
}

void loop() {
  if (BOOTSEL) {
    Serial.println("BARREL ROLL!!!");
    float r = 100;
    float ox = 0.0;
    float oy = 0.0;
    for (float a = 0.0f; a < 2.0f * 3.14159f; a += 0.1f) {
      float ax = r * cos(a);
      float ay = r * sin(a);
      float dx = ax - ox;
      float dy = ay - oy;
      MouseBT.move(dx, dy, 0);
      ox = ax;
      oy = ay;
      delay(10);
    }
    while (BOOTSEL) {
      delay(1);
    }
  }
}
