/*
   I2S bi-directional input and output loopback example
   Released to the Public Domain by Cooper Dalrymple
*/

#include <I2S.h>

I2S i2s(INPUT_PULLUP);

void setup() {
  Serial.begin(115200);

  i2s.setDOUT(0);
  i2s.setDIN(1);
  i2s.setBCLK(2); // Note: LRCLK = BCLK + 1
  i2s.setBitsPerSample(16);
  i2s.setFrequency(22050);
  // NOTE: The following values are known to work with the Adafruit microphone:
  // i2s.setBitsPerSample(32);
  // i2s.setFrequency(16000);
  i2s.begin();

  while (1) {
    int16_t l, r;
    i2s.read16(&l, &r);
    i2s.write16(l, r);
    // NOTE: Adafruit microphone word size needs to match the BPS above.
    // int32_t l, r;
    // i2s.read32(&l, &r);
    // i2s.write32(l, r);
  }
}

void loop() {
  /* Nothing here */
}
