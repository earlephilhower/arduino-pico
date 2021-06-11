/* Simple annoying siren example using tone() */
/* Released to the public domain by Earle F. Philhower, III */

#define TONEPIN 7

void setup() {
}

void loop() {
  for (int i = 100; i < 10000; i += 1) {
    tone(TONEPIN, i);
    delayMicroseconds(20);
  }
}
