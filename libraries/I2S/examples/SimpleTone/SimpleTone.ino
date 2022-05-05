/*
  This example generates a square wave based tone at a specified frequency
  and sample rate. Then outputs the data using the I2S interface to a
  MAX08357 I2S Amp Breakout board.

  created 17 November 2016
  by Sandeep Mistry
  modified for RP2040 by Earle F. Philhower, III <earlephilhower@yahoo.com>


    bool setBCLK(pin_size_t pin);
    - This assigns two adjacent pins - the pin after this one (one greater)
      is the WS (word select) signal, which toggles before the sample for
      each channel is sent

    bool setDATA(pin_size_t pin);
    - Sets the DOUT pin, can be any valid GPIO pin
*/

#include <I2S.h>

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

// GPIO pin numbers
#define pBCLK 20
#define pWS (pBCLK+1)
#define pDOUT 22

const int frequency = 440; // frequency of square wave in Hz
const int amplitude = 500; // amplitude of square wave
const int sampleRate = 16000; // minimum for UDA1334A

const int halfWavelength = (sampleRate / frequency); // half wavelength of square wave

int16_t sample = amplitude; // current sample value
int count = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  Serial.begin(115200);
  Serial.println("I2S simple tone");

  i2s.setBCLK(pBCLK);
  i2s.setDATA(pDOUT);
  i2s.setBitsPerSample(16);

  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
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
