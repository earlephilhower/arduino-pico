/*
  This example demonstrates I2S output with the optional MCLK  signal.
  Note: System clock sppeds used here may not be compatible with all other libraries
  requiring specific sys_clks, particularly Pico PIO USB.

  Original  17 November 2016
  by Sandeep Mistry
  modified for RP2040 by Earle F. Philhower, III <earlephilhower@yahoo.com>

    bool setBCLK(pin_size_t pin);
    - This assigns two adjacent pins - the pin after this one (one greater)
      is the WS (word select) signal, which toggles before the sample for
      each channel is sent

    bool setDATA(pin_size_t pin);
    - Sets the DOUT pin, can be any valid GPIO pin


    modified for MCLK by Richard Palmer June 2023
*/

#include <I2S.h>

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

// GPIO pin numbers
#define pDOUT 19
#define pBCLK 20
#define pWS (pBCLK+1)
#define pMCLK 22  // optional MCLK pin

const int frequency = 440; // frequency of square wave in Hz
const int sampleRate = 16000; // minimum for many i2s DACs
const int bitsPerSample = 16;
const int amplitude = 1 << (bitsPerSample - 2); // amplitude of square wave = 1/2 of maximum

#define MCLK_MUL  256     // depends on audio hardware. Suits common hardware.

const int halfWavelength = sampleRate / (2 * frequency); // half wavelength of square wave

int16_t sample = amplitude; // current sample value
int count = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  // set the system clock to a compatible value to the samplerate
  i2s.setSysClk(sampleRate); // best to do this before starting anything clock-dependent

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("I2S with MCLK - square wave");

  i2s.setBCLK(pBCLK);
  i2s.setDATA(pDOUT);
  i2s.setMCLK(pMCLK); // must be run before setFrequency() and i2s.begin()
  i2s.setMCLKmult(MCLK_MUL); // also before i2s.begin()
  i2s.setBitsPerSample(16);

  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (100); // do nothing
  }

}

void loop() {
  if (count % halfWavelength == 0) {
    // invert the sample every half wavelength count multiple to generate square wave
    sample = -1 * sample;
  }

  // write the same sample twice, once for left and once for the right channel
  i2s.write(sample);
  i2s.write(sample);

  // increment the counter for the next sample
  count++;
}
