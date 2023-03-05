/*
   I2S stereo microphone (input) example
   Run using the Arduino Serial Plotter to see waveform.
   Released to the Public Domain by Earle F. Philhower, III

   For the Google AIY Voice Hat Microphone daughterboard, part
   of the Raspberry Pi AIY cardboard box, the I2S stereo pinout
   looking at the board top with the RPI logo on the left hand
   side:
            +--   ------------------------------------  --+
   left RPI | (1) GND (2) DIN (3) BCLK (4) LRCLK (5) 3.3V | AIY right
       logo +---------------------------------------------+ logo

   For an Adfruit I2S MEMS microphone (https://www.adafruit.com/product/3421),
   connect the pins as follows:

      DOUT -> GPIO0
      BCLK <- GPIO1
      LRCL <- GPIO2  # LRCLK = BCLK + 1
      GND <-> GND
      3V  <-> 3V3OUT

   The other idiosyncrasy of most modern MEMS microphones is that they
   require a minimum clock rate to wake up. For example, the SPH0645
   microphone needs at least 1MHz.
*/

#include <I2S.h>

I2S i2s(INPUT);

void setup() {
  Serial.begin(115200);

  i2s.setDATA(0);
  i2s.setBCLK(1); // Note: LRCLK = BCLK + 1
  i2s.setBitsPerSample(16);
  i2s.setFrequency(22050);
  // NOTE: The following values are known to work with the Adafruit microphone:
  // i2s.setBitsPerSample(32);
  // i2s.setFrequency(16000);
  i2s.begin();

  while (1) {
    int16_t l, r;
    i2s.read16(&l, &r);
    // NOTE: Adafruit microphone word size needs to match the BPS above.
    // int32_t l, r;
    // i2s.read32(&l, &r);
    Serial.printf("%d %d\r\n", l, r);
  }
}

void loop() {
  /* Nothing here */
}
