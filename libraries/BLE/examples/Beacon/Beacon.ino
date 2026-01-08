#include <BLE.h>

// Starts a BLE Beacon in the background, no further app work required after begin()

BLEBeacon beacon;
void setup() {
  BLE.begin();

  // Mash up the board ID and a common header for our "UUID"
  uint8_t uuid[16];
  memcpy(uuid, "PICOBCON", 8);
  pico_get_unique_board_id((pico_unique_board_id_t *)(uuid + 8));

  // Define the beacon and start it
  beacon.setUUID(BLEUUID(uuid));
  beacon.setMajorMinor(6, 7);
  beacon.setTXPower(-44); // -44dbm @ 1M
  beacon.begin();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // We can do whatever app we want, the beacon is all handled automatically.
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}
