/* Earle F. Philhower, III <earlephilhower@yahoo.com>
   Benjamin Aigner <beni@asterics-foundation.org> <aignerb@technikum-wien.at> */
/* Released to the public domain */

#define ENABLE_LOG_INFO
#define ENABLE_LOG_DEBUG

#define USE_MOUSE
#define USE_KBD
#define USE_JOYSTICK

#ifdef USE_MOUSE
#include <MouseBLE.h>
#endif
#ifdef USE_KBD
#include <KeyboardBLE.h>
#endif
#ifdef USE_JOYSTICK
#include <JoystickBLE.h>
#endif

void setup() {
  Serial.begin(115200);

  //If activated nothing happens until the serial port is opened
  //while(!Serial);


#if (defined(USE_KBD) || defined(USE_JOYSTICK)) && defined(USE_MOUSE)
  MouseBLE.begin("BLE Composite");
#elif defined(USE_MOUSE)
  MouseBLE.begin("BLE Mouse");
#endif

#ifdef USE_KBD
  KeyboardBLE.begin("BLE KBD");
#endif

#ifdef USE_JOYSTICK
  JoystickBLE.begin("BLE JOY");
#endif

  Serial.printf("Press BOOTSEL to start action\n");
#ifdef USE_MOUSE
  Serial.println("First the mouse moves");
#endif
#ifdef USE_KBD
  Serial.println("Then \"Hi\" will be printed");
#endif
#ifdef USE_JOYSTICK
  Serial.println("Then joystick buttons & axis are changed");
#endif
}

void loop() {
  if (BOOTSEL) {
#ifdef USE_MOUSE
    Serial.println("ACTION!!!");
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
#endif

#ifdef USE_KBD
    KeyboardBLE.print("Hi");
#endif

#ifdef USE_JOYSTICK
    JoystickBLE.button(1, true);
    JoystickBLE.X(0);
    JoystickBLE.send_now();
    delay(1000);

    JoystickBLE.button(1, false);
    JoystickBLE.X(512);
    JoystickBLE.send_now();
#endif

    while (BOOTSEL) {
      delay(1);
    }
  }
}
