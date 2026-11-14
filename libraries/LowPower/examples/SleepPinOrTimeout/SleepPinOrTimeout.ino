// Light sleep until a button (GP15 to ground) is pressed or 10 seconds have passed, whichever comes first.
// Works the same with a radio module's IRQ line instead of the button.  Reports over Serial1 (GP0 TX / GP1 RX on a Pico).
#include <LowPower.h>

const pin_size_t WAKE_GPIO = 15;

void setup() {
  Serial1.begin(115200);
  pinMode(WAKE_GPIO, INPUT_PULLUP);
  delay(100);
}

void loop() {
  Serial1.println("sleeping");
  Serial1.flush(); // let the UART drain; its clock is gated while sleeping
  int rc = LowPower.sleepUntilPin(WAKE_GPIO, FALLING, 10000);
  if (rc != 0) {
    Serial1.printf("sleep failed: %d\n", rc);
  } else if (LowPower.wakeReason() == LowPower.WAKE_PIN) {
    Serial1.printf("woke by the pin after %lu ms\n", (unsigned long)LowPower.lastSleepMs());
  } else {
    Serial1.printf("woke by the timeout after %lu ms\n", (unsigned long)LowPower.lastSleepMs());
  }
  delay(500); // button debounce / let the line settle
}
