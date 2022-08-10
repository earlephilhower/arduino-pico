// This example overwrites itself with a serial blinker sketch using OTA
// In general, you will get a file from the Internet, over a serial port, etc.
// and not include it in a header like we do here for simplicity.
//
// The blinker.BIN file was compressed with `gzip -9` and will be expanded
// during OTA
//
// You need to have at least 256K of filesystem configured in
// Tools->Flash Size
//
// Released to the public domain August 2022 by Earle F. Philhower, III

#include <PicoOTA.h>
#include <LittleFS.h>
#include "blink_100_1000.h"

void setup() {
  Serial.begin(115200);
  delay(5000);

  Serial.printf("Writing OTA image of blinker...");
  LittleFS.begin();
  File f = LittleFS.open("blink.bin.gz", "w");
  if (sizeof(blink_gz) != f.write(blink_gz, sizeof(blink_gz))) {
    Serial.printf("Unable to write OTA binary.  Is the filesystem size set?\n");
    return;
  }
  f.close();
  Serial.printf("done\n\n");
  Serial.printf("Programming OTA commands...");
  picoOTA.begin();
  picoOTA.addFile("blink.bin.gz");
  picoOTA.commit();
  LittleFS.end();
  Serial.printf("done\n\n");
  Serial.printf("Rebooting in 5 seconds, should begin blinker instead of this app...\n");
  delay(5000);
  rp2040.reboot();
}

void loop() {
}
