/* Earle F. Philhower, III <earlephilhower@yahoo.com> */
/* Released to the public domain */

#define ENABLE_LOG_INFO
#define ENABLE_LOG_DEBUG
#include <MouseBLE.h>
#include <KeyboardBLE.h>
#include <JoystickBLE.h>

void setup() {
  Serial.begin(115200);
  MouseBLE.begin("BLE Composite");
  KeyboardBLE.begin("");
  JoystickBLE.begin("");
  delay(5000);
  Serial.printf("Press BOOTSEL to move the mouse in a circle\n");
  Serial.printf("Afterwards \"Hi\" will be printed and the joystick axis are moved\n");
}

void loop() {
  if (BOOTSEL) {
    Serial.println("BARREL ROLL!!!");
    float r = 100;
    float ox = 0.0;
    float oy = 0.0;
    for (float a = 0; a < 2.0 * 3.14159; a += 0.1) {
      float ax = r * cos(a);
      float ay = r * sin(a);
      float dx = ax - ox;
      float dy = ay - oy;
      MouseBLE.move(dx, dy, 0);
      ox = ax;
      oy = ay;
      delay(10);
    }
    MouseBLE.setBattery(random(0, 101)); // Set between 0...100%
    delay(1000);
    
    KeyboardBLE.print("Hi");
    
    delay(1000);
    
    JoystickBLE.button(1,true);
    JoystickBLE.X(0);
    JoystickBLE.send_now();
    delay(1000);
    
    JoystickBLE.button(1,false);
    JoystickBLE.X(512);
    JoystickBLE.send_now();
    
    while (BOOTSEL) {
      delay(1);
    }
  }
}
