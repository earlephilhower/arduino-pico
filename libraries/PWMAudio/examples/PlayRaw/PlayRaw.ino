/*
  This example plays a raw, headerless, mono 16b, 44.1 sample using the PWMAudio library on GPIO 1.

  Released to the public domain by Earle F. Philhower, III <earlephilhower@yahoo.com>
*/

#include <PWMAudio.h>
#include "wav.h"

// The sample pointers
const int16_t *start = (const int16_t *)out_raw;
const int16_t *p = start;

// Create the PWM audio device on GPIO 1.   Hook amp/speaker between GPIO1 and convenient GND.
PWMAudio pwm(1);

unsigned int count = 0;

void cb() {
  while (pwm.availableForWrite()) {
    pwm.write(*p++);
    count += 2;
    if (count >= sizeof(out_raw)) {
      count = 0;
      p = start;
    }
  }
}

void setup() {
  pwm.onTransmit(cb);
  pwm.begin(44100);
}

void loop() {
  /* noop, everything is done in the CB */
}
