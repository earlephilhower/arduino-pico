// Run a simple BLE scan and dump anything found, over and over

#include <BLE.h>

void setup() {
  delay(5000);
  BLE.begin(""); // Name not important, we're not going to advertise
  Serial.println("Bluetooth Low Energy started");
}

void loop() {
  Serial.println("Starting scan...");
  BLEScanReport *report = BLE.scan(10);
  for (auto item : *report) {
    Serial.println(item.toString());
  }
  Serial.println("DONE");
}

