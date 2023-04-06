#include <BTstackLib.h>
#include <SPI.h>

/* EXAMPLE_START(iBeaconScanner): iBeacon Scanner

   @section Setup

   @text After BTstack.setup(), BTstack is configured to call
   advertisementCallback whenever an Advertisement was received.
   Then, a device discovery is started
*/

/* LISTING_START(iBeaconSetup): iBeacon Scanner Setup */
void setup(void) {
  Serial.begin(9600);
  BTstack.setup();
  BTstack.setBLEAdvertisementCallback(advertisementCallback);
  BTstack.bleStartScanning();
}
/* LISTING_END(iBeaconSetup): iBeacon Scanner Setup */

void loop(void) {
  BTstack.loop();
}

/*
   @section Advertisement Callback

   @text Whenever an Advertisement is received, isIBeacon() checks if
   it contains an iBeacon. If yes, the Major ID, Minor ID, and UUID
   is printed.
   If it's not an iBeacon, only the BD_ADDR and the received signal strength
   (RSSI) is shown.
*/
/* LISTING_START(iBeaconCallback): iBeacon Scanner Callback */
void advertisementCallback(BLEAdvertisement *adv) {
  if (adv->isIBeacon()) {
    Serial.print("iBeacon found ");
    Serial.print(adv->getBdAddr()->getAddressString());
    Serial.print(", RSSI ");
    Serial.print(adv->getRssi());
    Serial.print(", UUID ");
    Serial.print(adv->getIBeaconUUID()->getUuidString());
    Serial.print(", MajorID ");
    Serial.print(adv->getIBeaconMajorID());
    Serial.print(", MinorID ");
    Serial.print(adv->getIBecaonMinorID());
    Serial.print(", Measured Power ");
    Serial.println(adv->getiBeaconMeasuredPower());
  } else {
    Serial.print("Device discovered: ");
    Serial.print(adv->getBdAddr()->getAddressString());
    Serial.print(", RSSI ");
    Serial.println(adv->getRssi());
  }
}
/* LISTING_END(iBeaconCallback): iBeacon Scanner Callback */

