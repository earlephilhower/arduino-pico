// Brute force SYSCLK checker for I2S.  Counts down from F_CPU and
// calculates the absolute I2S speed error for a given frequency,
// reporting the minimum encountered and at which frequency
// (Note 64K is 32K bidirectional, see #3496)
// Run it on any new CPUs or updated default F_CPUs.

// End users should not need to ever run this
// Released to the public domain 2026 by Earle F. Philhower, III

void setup() {
  delay(5000);
  int F[11] = { 8000, 16000, 32000, 64000, 48000, 96000, 192000, 11025, 22050, 44100, 88200 };
  for (auto x = 0; x < 11; x++) {
    int f = F[x];
    Serial.printf("Checking for %d\n", f);
    int khz = (F_CPU * 1.10) / 1000;
    float err = 999999.9;
    int bestMatch = khz;
    while (khz > 100000) {
      uint a, b, c;
      bool r = check_sys_clock_khz(khz, &a, &b, &c);
      if (!r) {
        khz--;
        continue;
      }
      float edgerate = f * 64; // 32 bits, high and low events
      float v = (khz * 1000) / edgerate;
      float e = v - floor(v);
      e /= v;
      e *= 100.0;
      if (e < err) {
        err = e;
        bestMatch = khz;
        Serial.printf("%d %f\n", bestMatch, err);
        if (err == 0) {
          break;
        }
      }
      khz--;
    }
    Serial.printf("%d: %d, err=%1.6f%%\n", f, bestMatch, err);
  }
}

void loop() {
}
