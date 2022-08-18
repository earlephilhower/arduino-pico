/* Simple sketch to do something on a BOOTSEL press */
/* Released into the public domain */

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("I dare you to hit the BOOTSEL button...");
}

int c = 0;
void loop() {
  if (BOOTSEL) {
    Serial.printf("\a\aYou pressed BOOTSEL %d times!\n", ++c);
    // Wait for BOOTSEL to be released
    while (BOOTSEL) {
      delay(1);
    }
  }
}
