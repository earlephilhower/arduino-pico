// Go dormant until GP15 goes low (connect GP15 to GND). Reports over Serial1 (GP0/GP1 on a Pico).
// NOTE: millis() stops while dormant. All pins except GP15 and the UART/I2C pins are parked in a
// low-leakage state while dormant and restored on wake (LowPower.setPinRestore(false) keeps them parked).
#include <LowPower.h>

const pin_size_t BUTTON_PIN = 15;

void setup() {
  Serial1.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(100);
}

void loop() {
  Serial1.printf("going dormant: millis=%lu\n", (unsigned long)millis());
  Serial1.flush();
  int rc = LowPower.dormantUntilPin(BUTTON_PIN, FALLING);
  uint32_t slept = LowPower.lastSleepMs(); // 0xFFFFFFFF = unknown (pin wake has no time reference)
  Serial1.printf("woke: rc=%d millis=%lu lastSleepMs=%lu\n", rc, (unsigned long)millis(), (unsigned long)slept);
  delay(2000);
}
