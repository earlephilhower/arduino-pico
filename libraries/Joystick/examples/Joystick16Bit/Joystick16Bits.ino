#include <Joystick.h>

// Set 1KHz polling frequency (1000/second)
int usb_hid_poll_interval = 1;

void setup() {
  Joystick.use16bit();
  Joystick.begin();
}

void loop() {
  static int16_t delta = 1;
  static int16_t p = 0;
  if (p == 32767) {
    delta = -1;
  } else if (p == -32767) {
    delta = 1;
  }
  p += delta;
  Joystick.X(p);
  Joystick.Y(-p);
}
