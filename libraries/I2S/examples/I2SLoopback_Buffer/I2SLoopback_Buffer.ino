/*
   I2S bi-directional input and output buffered loopback example
   Released to the Public Domain by Cooper Dalrymple
*/

#include <I2S.h>

I2S i2s(INPUT_PULLUP);

#define SIZE 256
int16_t buffer[SIZE];

void setup() {
  Serial.begin(115200);

  i2s.setDOUT(0);
  i2s.setDIN(1);
  i2s.setBCLK(2); // Note: LRCLK = BCLK + 1
  i2s.setBitsPerSample(16);
  i2s.setFrequency(22050);
  i2s.setBuffers(6, SIZE * sizeof(int16_t) / sizeof(uint32_t));
  i2s.begin();

  size_t count, index;
  while (1) {
    count = i2s.read((uint8_t *)&buffer, SIZE * sizeof(int16_t)) * sizeof(uint32_t) / sizeof(int16_t);
    index = 0;
    while (index < count) {
      // Reduce volume by half
      buffer[index++] >>= 1; // right
      buffer[index++] >>= 1; // left
    }
    i2s.write((const uint8_t *)&buffer, count * sizeof(int16_t));
  }
}

void loop() {
  /* Nothing here */
}
