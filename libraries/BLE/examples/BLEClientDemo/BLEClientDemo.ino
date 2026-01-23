// Demonstrates BLEClient usage (and stress test)

// To use:
// Run a copy of CharacteristicOnWrite on another PicoW
// Then run this sketch on a separate PicoW to demonstrate
// 1. Scanning for specific BLE services
// 2. Automatically connecting to the remote PicoW
// 3. Reading characteristic values like name and "uptime"
// 4. Enable BLE notifications and receive them automatically when remote values change
// 5. Writing to the LED characteristic to blink the LED


#include <BLE.h>

// From the CharacteristicOnWrite sketch
const char *myserviceuuid = "c3feed70-b50c-400a-836c-c8981beb0b1c";
const char *leduuid = "c3feed71-b50c-400a-836c-c8981beb0b1c";
const char *uptimeuuid = "c3feed72-b50c-400a-836c-c8981beb0b1c";

void setup() {
  // Start BLE without a name since we're going to be a client, not server
  BLE.begin();
  Serial.println("Bluetooth Low Energy started");
}

// The notify callback.  Will get an interrupt-driven call when the attached
// remote characteristic changes.
void notify(BLERemoteCharacteristic *c, const uint8_t *data, uint32_t len) {
  (void) c;
  Serial.print("Notify: ");
  Serial.write(data, len);
  Serial.println();
}

uint32_t cnt = 1; // loop count

void loop() {
  Serial.printf("Starting scan %d...free heap %d\n", cnt++, rp2040.getFreeHeap());

  // Scan for all available BLE servers with my custom UUID
  auto report = BLE.scan(BLEUUID(myserviceuuid), 5);

  // Print them out to screen (normally only 1 Pico present)
  int i = 0;
  for (auto item : *report) {
    Serial.print(i++);
    Serial.print(' ');
    Serial.println(item.toString());
  }
  Serial.println("DONE");

  if (report->empty()) {
    Serial.println("No PicoW Servers found, retrying in 10 seconds");
    delay(10000);
    return;
  }

  // Select the first server in the list
  BLEAdvertising a = report->front();
  Serial.print("Connecting to: ");
  Serial.println(a.toString());

  // And connect to it
  BLE.client()->connect(a, 10);

  // Read out the device name (service 0x1800, characteristic 0x2a00)
  auto svc = BLE.client()->service(BLEUUID(0x1800));
  if (svc) {
    auto c = svc->characteristic(BLEUUID(0x2a00));
    if (c) {
      Serial.printf("name: %s\n", c->getString().c_str());
    }
  }

  // Now useto my custom service UUID
  svc = BLE.client()->service(BLEUUID(myserviceuuid));
  if (svc) {

    // Start a notifier and enable callbacks on the Uptime characteristic
    auto upt = svc->characteristic(BLEUUID(uptimeuuid));
    if (upt) {
      Serial.println("Adding uptime notifier");
      upt->onNotify(notify);
      upt->enableNotifications();
    }

    // Examine the LED user description (human readable text)
    auto led = svc->characteristic(BLEUUID(leduuid));
    if (led) {
      Serial.printf("LED description '%s'\n", led->getDescription().c_str());
      // And toggle the remote LED, it's a 1-byte characteristic
      led->setValue(true);
      delay(1000);
      led->setValue(false);
      delay(1000);
    }

    // Use normal read on the uptime characteristic
    if (upt) {
      Serial.print("Characteristic read uptime: ");
      Serial.println(upt->getString());
    }
  }

  // Wait a bit, we'll get notifications every second while we delay()
  delay(10000);

  // Disconnect from the device so we start fresh on next loop
  BLE.client()->disconnect();
}
