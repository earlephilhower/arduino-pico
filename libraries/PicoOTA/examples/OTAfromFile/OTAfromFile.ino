// This example overwrites itself with a blinker (for the Pico only) using OTA
// In general, you will get a file from the Internet, over a serial port, etc.
// and not include it in a header like we do here for simplicity.
//
// To run, you need to have a standard Pico (since the included blink binary is
// hardcoded  to flash the Pico's LED.
//
// You also need to have at least 256K of filesystem configured in
// Tools->Flash Size
//
// Released to the public domain July 2022 by Earle F. Philhower, III

#include <PicoOTA.h>
#include <LittleFS.h>
#include "blink_100_1000.h"

void setup() {
  Serial.begin(115200);
  delay(5000);
  
  Serial.printf("Writing OTA image of blinker...");
  LittleFS.begin();
  File f = LittleFS.open("blink.bin", "w");
  if (sizeof(blink_100_1000) != f.write(blink_100_1000, sizeof(blink_100_1000))) {
    Serial.printf("Unable to write OTA binary.  Is the filesystem set?\n");
    return;
  }
  f.close();
  Serial.printf("done\n\n");
  Serial.printf("Programming OTA commands...");
  picoOTA.begin();
  picoOTA.addFile("blink.bin");
  picoOTA.commit();
  LittleFS.end();
  Serial.printf("done\n\n");
  Serial.printf("Rebooting in 5 seconds, should begin blinker instead of this app...\n");
  delay(5000);
  rp2040.reboot();
}

void loop() {
}
