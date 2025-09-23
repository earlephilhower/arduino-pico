// Simple sketch showing how to set the USB information from an application
// Released to the public Domain 2025 by Earle F. Philhower, III

#include <USB.h>

void setup() {
  // Must *always* disconnect the USB port while doing modifications like this
  USB.disconnect();
  USB.setVIDPID(0x1209, 0xca75);
  USB.setManufacturer("Kitties 'R Us");
  USB.setProduct("CAT Scanner");
  USB.setSerialNumber("FUZZBALL_0001");
  // Everything is set, so reconnect under the new device info
  USB.connect();
}

void loop() {
  Serial.println("Look at the USB devices/lsusb output to see the new device!");
  delay(1000);
}
