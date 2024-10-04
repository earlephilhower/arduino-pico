// This sketch will cycle thru some possible restart reasons and shall help you understanding the rp2040.getResetReason() function
// Author: palmerr23

#include <EEPROM.h>
#define NTESTS 5  // Number of tests, Debug port test not enabled
char resetReasonText[][24] = { "Unknown", "Power On / Brownout", "Run pin", "Software", "Watchdog Timer", "Debug reset" };
char testText[][128] = { "Run pin\n\tShort Run pin to ground", "Power On / Brownout\n\tDisconnect / reconnect power (i.e. USB)", "Software reBOOT in 5 Secs", "Watchdog Timer in 7 Secs", "Software reSTART in 5 Secs\n\tUSB Serial does not always restart", "Debug port - do something!" };

// Small function that will send one dot every second over Serial, forever.
void delayCount(void) {
  int t = 1;
  while (1) {
    Serial.print(".");
    if (t % 50 == 0) {
      Serial.println();
    }
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  delay(1000);
  Serial.println("Reset reason:");

  RP2040::resetReason_t rr = rp2040.getResetReason();
  Serial.printf("RR %i %s\n\n", rr, resetReasonText[rr]);

  EEPROM.begin(512);
  byte test = EEPROM.read(0);
  if (test >= NTESTS) {  // un-initialised EEPROM read is random
    test = -1;
  }
  test = (test + 1) % NTESTS; // Go to next test, but limit to NTESTS-1
  Serial.printf("Test %i: %s\n", test, testText[test]);
  EEPROM.write(0, test);
  EEPROM.commit();
  delay(5000);
  switch (test) {
    case 0:  //  Run pin
      delayCount();
      break;  // Break to avoid compiler warnings
    case 1:   // Power On
      delayCount();
      break;
    case 2:  // reboot command
      rp2040.reboot();
      delayCount();
      break;
    case 3:  // watchdog
      rp2040.wdt_begin(2000);
      rp2040.wdt_reset();
      delayCount();
      break;
    case 4:  // restart command
      rp2040.restart();
      delayCount();
      break;
  }
}

// never gets here
void loop() {
  delay(100);
}