// Keep a PWM output running while the chip sleeps: 1 kHz, 25 % duty on GP15 during a 10 second sleepFor().
// keepRunning() leaves the PWM clock and the system clock PLL on during the sleep (about 7 mA board current on a Pico 2 W instead of about 2.6 mA,
// without keepRunning the PLL is off).  Output on Serial1 (GP0 TX).
#include <LowPower.h>

const pin_size_t PWM_PIN = 15;

void setup() {
  Serial1.begin(115200);
  delay(100);
  analogWriteFreq(1000);
  analogWriteRange(255);
  analogWrite(PWM_PIN, 64);
}

void loop() {
  Serial1.println("sleeping 10 s with the PWM running");
  Serial1.flush();
  LowPower.keepRunning(LowPowerClass::KR_PWM);
  LowPower.sleepFor(10000);
  LowPower.keepRunning(0);  // back to the normal gated sleep
  Serial1.println("awake");
  delay(1000);
}
