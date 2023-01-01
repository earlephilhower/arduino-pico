/*
  This example plays a stereo signal, low frequency on the left and high frequency on the right

  Released to the public domain by Earle F. Philhower, III <earlephilhower@yahoo.com>
*/

#include <PWMAudio.h>

PWMAudio pwm(0, true); // GP0 = left, GP1 = right

const int freq = 44100; // Output frequency for PWM
const int ampl = 1500; // How loud (32767 == loudest)

unsigned int cnt = 0;

void cb() {
  static int l, r;
  static int16_t al = ampl;
  static int16_t ar = ampl;
  while (pwm.availableForWrite()) {
    if (++l == freq / 440 / 2) {
      al = -al;
      l = 0;
    }
    if (++r == freq / 3520 / 2) {
      ar = -ar;
      r = 0;
    }
    pwm.write(al);
    pwm.write(ar);
    cnt++;
  }
}

void setup() {
  pwm.onTransmit(cb);
  pwm.begin(freq);
}

void loop() {
}
