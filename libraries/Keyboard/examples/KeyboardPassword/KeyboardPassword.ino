/* Released into the public domain */
/* Earle F. Philhower, III <earlephilhower@yahoo.com> */

#include <Keyboard.h>

void ledCB(bool numlock, bool capslock, bool scrolllock, bool compose, bool kana, void *cbData) {
  (void) numlock;
  (void) scrolllock;
  (void) compose;
  (void) kana;
  (void) cbData;
  digitalWrite(LED_BUILTIN, capslock ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Keyboard.onLED(ledCB);
  Keyboard.begin();
  delay(5000);
  Serial.printf("Arduino USB Password Typer\n");
  Serial.printf("Press BOOTSEL to enter your super-secure(not!) password\n\n");
}

void loop() {
  if (BOOTSEL) {
    Serial.println("Typing password for you...shhhh....");

    Serial.println("First, mute the computer to be extra quiet");
    Keyboard.consumerPress(KEY_MUTE);
    delay(100);
    Keyboard.consumerRelease();

    Keyboard.print("ThisPasswordIsWeakLikeABaby");

    Serial.println("OK, unmute the computer since our secret is done");
    Keyboard.consumerPress(KEY_MUTE);
    delay(100);
    Keyboard.consumerRelease();

    while (BOOTSEL);
  }
}
