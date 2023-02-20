/*
  This example plays a tune that alternates between left and right channels using a simple sine wave.

  Released to the public domain by Earle F. Philhower, III <earlephilhower@yahoo.com>
*/

#include <PWMAudio.h>

PWMAudio pwm(0, true); // GP0 = left, GP1 = right

const int freq = 48000; // Output frequency for PWM

int16_t al = 0;
int16_t ar = 0;

const int notes[] = { 784, 880, 698, 349, 523 };
const int dly[] =   { 400, 500, 700, 500, 1000 };
const int noteCnt = sizeof(notes) / sizeof(notes[0]);

int freqL = 1;
int freqR = 1;

double sineTable[128]; // Precompute sine wave in 128 steps

unsigned int cnt = 0;
void cb() {
  while (pwm.availableForWrite()) {
    double now = ((double)cnt) / (double)freq;
    int fl = freqL << 7; // Prescale by 128 to avoid FP math later on
    int fr = freqR << 7; // Prescale by 128 to avoid FP math later on
    pwm.write((int16_t)(al * sineTable[(int)(now * fl) & 127]));
    pwm.write((int16_t)(ar * sineTable[(int)(now * fr) & 127]));
    cnt++;
  }
}

void setup() {
  // Set up sine table for waveform generation
  for (int i = 0; i < 128; i++) {
    sineTable[i] = sin(i * 2.0 * 3.14159 / 128.0);
  }
  pwm.setBuffers(4, 32); // Give larger buffers since we're are 48khz sample rate
  pwm.onTransmit(cb);
  pwm.begin(freq);
}

void loop() {
  delay(1000);

  // Send it out on the LHS
  ar = 0;
  for (int i = 0; i < noteCnt; i++) {
    freqL = notes[i];
    al = 5000;
    delay(dly[i]);
  }
  al = 0;
  delay(1000);

  // Hear reply on RHS
  for (int i = 0; i < noteCnt; i++) {
    freqR = notes[i] / 4;
    ar = 15000;
    delay(dly[i]);
  }
  ar = 0;
  delay(3000);
}
