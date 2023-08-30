/*
  This example just sends out TDM data on 8 channels with their value
  equal to the channel number.
  Released to the public domain by Earle F. Philhower, III <earlephilhower@yahoo.com>
*/

#include <I2S.h>

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

// GPIO pin numbers
#define pBCLK 20
#define pWS (pBCLK+1)
#define pDOUT 22

const int sampleRate = 1000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  Serial.begin(115200);
  Serial.println("I2S TDM test");

  i2s.setBCLK(pBCLK);
  i2s.setDATA(pDOUT);
  i2s.setBitsPerSample(32);
  i2s.setTDMFormat();
  i2s.setTDMChannels(8);

  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }

}

void loop() {
  uint32_t x = 0;
  while (1) {
    i2s.write(x & 0x07);
    if (!(x % 10000)) {
      Serial.println(x);
    }
    x++;
  }
}
