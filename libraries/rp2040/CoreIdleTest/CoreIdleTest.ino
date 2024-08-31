// Simple test of core freezing. Applications should not have a reason to
// freeze a core, but LittleFS and EEPROM need to be able to do it to
// avoid crashes when the opposite core tries to read flash while it's
// being programmed.
// Public domain 2024, Earle F. Philhower

volatile int c0 = 0;
volatile int c1 = 0;
void setup() {
  delay(5000);
  for (int i = 0; i < 200; i++) {
    c0++;
    Serial.printf("%d %d\n", c0, c1);
    delay(1);
  }
  int start = c1;
  rp2040.idleOtherCore();
  for (int i = 0; i < 2000; i++) {
    c0++;
    delay(1);
  }
  int end = c1;
  rp2040.resumeOtherCore();
  Serial.printf("Core0: %d, Core1 now %d, Core1 delta while stopped: %d\n", c0, c1, end - start);
  for (int i = 0; i < 200; i++) {
    c0++;
    Serial.printf("%d %d\n", c0, c1);
    delay(1);
  }

}

void loop() {
}

void setup1() {
  delay(5000);
  while (1) {
    c1++;
    delay(1);
  }
}

void loop1() {
}
