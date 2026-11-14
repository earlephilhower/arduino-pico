/*
  WiFiPowerOffSleep:
  Connect, do some network work, then power the WiFi chip off and sleep for a minute: the board
  draws about 4 mA (Pico 2 W) instead of about 50 mA with the chip connected.  Powering the
  chip on and re-joining takes about 7 s, so this pays off for sleeps longer than about 10 s.

  Pico W / Pico 2 W only.  Uses the LowPower library for the sleep (any other sleep or delay works too).

  This code is in the public domain.
*/

#include <WiFi.h>
#include <LowPower.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

void setup() {
  Serial1.begin(115200);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  Serial1.printf("connected, IP %s, RSSI %d dBm\n", WiFi.localIP().toString().c_str(), (int)WiFi.RSSI());
  // ... talk to the network here ...

  Serial1.println("WiFi off, sleeping for 60 s");
  Serial1.flush();
  WiFi.powerOff();           // leave the network, WL_ON low
  LowPower.sleepFor(60000);  // the chip is off: about 4 mA on a Pico 2 W
  bool ok = WiFi.powerOn();  // chip on, firmware load, join the same network again (blocks)
  Serial1.printf("WiFi back: %s\n", ok ? "connected" : "FAILED to reconnect");
  if (!ok) {
    delay(5000);  // try again on the next pass
  }
}
