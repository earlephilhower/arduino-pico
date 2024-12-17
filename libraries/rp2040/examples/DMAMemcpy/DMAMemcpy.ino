// Simple speed and functionality test for DMA memcpy
// Released to the public domain by Earle F. Philhower, III, 2024


uint32_t src[1024];
uint32_t dest[1024];

void setup() {
  // Set up a simple test pattern in src, verify after each pass
  srand(0xc0de);
  for (int i = 0; i < 1024; i++) {
    src[i] = ((~i) & 1024) | ((i) << 22);
  }
}

void verify(const char *name, uint32_t *src) {
  for (int i = 0; i < 1024; i++) {
    uint32_t expected = ((~i) & 1024) | ((i) << 22);
    if (expected != src[i]) {
      Serial.printf("ERROR, mismatch @ %d on %s memcpy\n", i, name);
      while (true) {
        // Idle forever, this is fatal!
      }
    }
  }
}

void loop() {
  uint64_t start, stop;

  start = rp2040.getCycleCount64();
  for (int i = 0; i < 1000; i++) {
    memcpy(dest, src, 4 * 1024);
    memcpy(src, dest, 4 * 1024);
  }
  stop = rp2040.getCycleCount64();
  verify("CPU", src);
  verify("CPU", dest);
  Serial.printf("CPU: %lld clock cycles for 4K\n", (stop - start) / 1000);

  start = rp2040.getCycleCount64();
  for (int i = 0; i < 1000; i++) {
    rp2040.memcpyDMA(dest, src, 4 * 1024);
    rp2040.memcpyDMA(src, dest, 4 * 1024);
  }
  stop = rp2040.getCycleCount64();
  verify("DMA", src);
  verify("DMA", dest);
  Serial.printf("DMA: %lld clock cycles for 4K\n\n\n", (stop - start) / 1000);
  delay(1000);
}
