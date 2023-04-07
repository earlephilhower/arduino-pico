#include <BTstackLib.h>
#include <stdio.h>
#include <SPI.h>

/* EXAMPLE_START(iBeacon): iBeacon Simulator

   @section Setup

   @text After BTstack.setup(), iBeaconConfigure() configures BTstack
   to send out iBeacons Advertisements with the provided Major ID,
   Minor ID and UUID.
*/
/* LISTING_START(iBeaconSetup): iBeacon Setup */
UUID uuid("E2C56DB5-DFFB-48D2-B060-D0F5A71096E0");
void setup(void) {
  Serial.begin(9600);
  BTstack.setup();
  BTstack.iBeaconConfigure(&uuid, 4711, 2);
  BTstack.startAdvertising();
}
/* LISTING_END(iBeaconSetup) */

void loop(void) {
  BTstack.loop();
}
