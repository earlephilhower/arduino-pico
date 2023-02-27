/* Released into the public domain */
/* Earle F. Philhower, III <earlephilhower@yahoo.com> */

#include <KeyboardBT.h>

void setup() {
  Serial.begin(115200);
  KeyboardBT.begin();
  delay(5000);
  Serial.printf("Arduino USB Password Typer\n");
  Serial.printf("Press BOOTSEL to enter your super-secure(not!) password\n\n");
}

void loop() {
  if (BOOTSEL) {
    Serial.println("Typing password for you...shhhh....");
    KeyboardBT.print("ThisPasswordIsWeakLikeABaby");
    while (BOOTSEL);
  }
}
