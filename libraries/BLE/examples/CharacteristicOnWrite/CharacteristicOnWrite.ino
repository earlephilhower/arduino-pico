// Implements a simple custom service and characteristics as well as
// showing how to hide/show the advertising (i.e. making the PicoW
// discoverable or not
//
// Press the BOOTSEL button to toggle the advertising
//
// Use nRF Connect or similar to read the uptime characteristic
// Use nRF Connect or similar to write a single byte 0/1 to control the LED


#include <BLE.h>


// We make a new custom service, uuidgen
BLEService service(BLEUUID("c3feed70-b50c-400a-836c-c8981beb0b1c"));

// We have a writable characteristic that users can control the LED with
BLECharacteristic led(BLEUUID("c3feed71-b50c-400a-836c-c8981beb0b1c"), BLEWrite, "LED State");

// And we have a readable (and notify-able) one with the Pico's uptime
BLECharacteristic uptime(BLEUUID("c3feed72-b50c-400a-836c-c8981beb0b1c"), BLERead | BLENotify, "Uptime in seconds");


bool on = true; // Advertising toggle
uint32_t u; // Last uptime change


void setup() {
  delay(5000);

  // Set the LED OFF
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // If we want to require bonding must set it before BLE.begin()
  // BLE.setSecurity(BLESecurityJustWorks);

  // Start BT and BLE with our name
  BLE.begin("PicoWsecure");

  // Characteristics need to belong to a Service, so add in LED and Uptime
  service.addCharacteristic(&led);
  service.addCharacteristic(&uptime);
  // Services need to be connected to a Server.  Multiple services are possible, just addService() them
  BLE.server()->addService(&service);

  // Prepare uptime value
  uptime.setValue("0");
  u = millis();

  // Prepare LED starting value
  led.setValue((uint8_t) 0);
  // On a write we get a callback.  This is a lambda and lets us write the function right inline with the call.
  // For more complicated operations, write a separate "void cbfunction(BLECharacteristic *c)" and pass it in here/
  led.onWrite([](BLECharacteristic * c) {
    digitalWrite(LED_BUILTIN, c->getBool());
  }
             );
  // It is also possible to set a class for the callbacks.  Implement a subclass of "class CharacteriscitCallback"
  // and use characteristic.setCallbacks(&myclassinstance).  This is used by the BLEServiceUART class to provide
  // a self-contained BLE service object.

  //Announce our presence
  BLE.startAdvertising();
}

void loop() {
  if (BOOTSEL) {
    while (BOOTSEL);
    if (on) {
      BLE.stopAdvertising();
      Serial.println("Stopping advertising");
    } else {
      BLE.startAdvertising();
      Serial.println("Starting advertising");
    }
    on = !on;
  }

  // We don't want to update 100s of times/sec, so wait until at least 1 second has passed before changing
  if (millis() - u > 1000) {
    u = millis();
    char b[32];
    sprintf(b, "%lu", u / 1000);
    uptime.setValue(String(b));
    Serial.printf("Uptime: %s\n", b);
  }
}
