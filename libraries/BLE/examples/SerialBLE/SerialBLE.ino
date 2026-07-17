#include <BLE.h>

BLEServiceUART uart;
BLEServiceBattery batt;

void setup() {
  delay(5000);
  BLE.begin("PicoUART");
  BLE.server()->addService(&batt);
  BLE.server()->addService(&uart);
  BLE.startAdvertising();
  uart.setAutoflush(50);
}

int cnt = 0;
void loop() {
  while (uart.available()) {
    Serial.println(uart.read());
  }
  uart.println(cnt++);
  uart.println("This text is very long and should require multiple writes to happen");
  delay(1000);
}
