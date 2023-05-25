#include <BTstackLib.h>
#include <SPI.h>

/*
   EXAMPLE_START(LEDeviceScanner): LE Device Scanner

   @text The LE Device Scanner monitors BLE device advertisements,
   keeping track of one or more known devices as they appear.
*/

// Application state
int counter[2] = {0, 0};
BD_ADDR known_devices[2] = {BD_ADDR("DB:88:B6:70:9E:EB"),
                            BD_ADDR("C9:60:BD:F2:B4:9D")
                           };

/*
   @section Setup

   @text After BTstack.setup(), BTstack is configured to call
   advertisementCallback whenever an Advertisement was received.
*/
/* LISTING_START(LEDeviceScannerSetup): LE Device Scanner Setup */
void setup(void) {
  Serial.begin(9600);
  BTstack.setBLEAdvertisementCallback(advertisementCallback);
  BTstack.setup();
  BTstack.bleStartScanning();
}
/* LISTING_END(LEDeviceScannerSetup): LE Device Scanner Setup */

/*
   @section Loop

   @text In the standard Arduino loop() function, BTstack's loop() is called.
*/
/* LISTING_START(LEDeviceScannerLoop): Loop */
void loop(void) {
  BTstack.loop();
}
/* LISTING_END(LEDeviceScannerLoop): Loop */

/*
   @section Advertisement Callback

   @text Whenever an Advertisement is received, isIBeacon() checks if
   it contains an iBeacon.

   If it's not an iBeacon, the BD_ADDR is compared to the address we are
   looking for and the counter is incremented.
*/
/* LISTING_START(LEDeviceScannerAdvertisementCallback): Advertisement Callback
*/
void advertisementCallback(BLEAdvertisement *bleAdvertisement) {
  if (!(bleAdvertisement->isIBeacon())) {
    Serial.print("Device discovered: ");
    Serial.println(bleAdvertisement->getBdAddr()->getAddressString());
    for (size_t i = 0; i < sizeof(counter) / sizeof(int); i++) {
      if (memcmp(bleAdvertisement->getBdAddr()->getAddress(),
                 known_devices[i].getAddress(), sizeof(known_devices[i])) == 0) {
        counter[i]++;
        Serial.printf("Known device: %s, has been discovered %d times.\n",
                      known_devices[i].getAddressString(), counter[i]);
      }
    }
  }
}
/* LISTING_END(LEDeviceScannerAdvertisementCallback): Advertisement Callback */
