// Hardware test for BLE notifications under FreeRTOS, driven by ../ble_notify_test.py.
// Build for a Pico W with FreeRTOS SMP and a Bluetooth IP/BT stack.
//
// loop() pushes a new counter value every 10ms, so each setValue() call takes the
// Bluetooth lock and sends a notification from a user task while a client is subscribed.

#include <BLE.h>

BLEService service(BLEUUID("c3feed70-b50c-400a-836c-c8981beb0b1c"));
BLECharacteristic led(BLEUUID("c3feed71-b50c-400a-836c-c8981beb0b1c"), BLEWrite, "LED State");
BLECharacteristic counter(BLEUUID("c3feed72-b50c-400a-836c-c8981beb0b1c"), BLERead | BLENotify, "Counter");

uint32_t u;
uint32_t n;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  BLE.begin("PicoWnotify");
  service.addCharacteristic(&led);
  service.addCharacteristic(&counter);
  BLE.server()->addService(&service);
  counter.setValue("0");
  led.setValue((uint8_t) 0);
  led.onWrite([](BLECharacteristic * c) {
    digitalWrite(LED_BUILTIN, c->getBool());
  });
  BLE.startAdvertising();
  u = millis();
}

void loop() {
  if (millis() - u > 10) {
    u = millis();
    char b[16];
    sprintf(b, "%lu", ++n);
    counter.setValue(String(b));
    if (!(n % 100)) {
      Serial.printf("Alive: %s\n", b);
    }
  }
}
