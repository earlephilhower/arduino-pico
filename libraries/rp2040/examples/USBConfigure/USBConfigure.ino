// Simple sketch showing how to set the USB information from an application
// Released to the public Domain 2025 by Earle F. Philhower, III

void setup() {
  // Must *always* disconnect the USB port while doing modifications like this
  usbDisconnect();
  usbSetVIDPID(0x1209, 0xca75);
  usbSetManufacturer("Kitties 'R Us");
  usbSetProduct("CAT Scanner");
  usbSetSerialNumber("FUZZBALL_0001");
  // Everything is set, so reconnect under the new device info
  usbConnect();
}

void loop() {
  Serial.println("Look at the USB devices/lsusb output to see the new device!");
  delay(1000);
}
