void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("High precision timing measurements/delays can be built using");
  Serial.println("rp2040.getCycleCount() and rp2040.getCycleCount64().\n");
  uint32_t a = rp2040.getCycleCount();
  delay(1000);
  uint32_t b = rp2040.getCycleCount();
  Serial.printf("There are %lu cycles in one second.\n\n\n", b - a);

  delay(3000);

  Serial.println("Note that the (32-bit) getCycleCount() will wraparound in ~30 seconds.");
  Serial.println("Using the 64-bit getCycleCount64() makes it practically infinite.");
  Serial.printf("\n%15s - %15s\n", "getCycleCount", "getCycleCount64");
}

void loop() {
  Serial.printf("%15lu - %15llu\n", rp2040.getCycleCount(), rp2040.getCycleCount64());
  delay(1500);
}
