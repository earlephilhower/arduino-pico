/*
   Mono analog microphone example using electret mike on A0
   Run using the Arduino Serial Plotter to see waveform.
   Released to the Public Domain by Earle F. Philhower, III

   Wire the mike's VCC to 3.3V on the Pico, connect the mike's
   GND to a convenient Pico GND, and then connect mike OUT to A0
*/

#include <ADCInput.h>

ADCInput mike(A0);
// For stereo/dual mikes, could use this line instead
//   ADCInput(A0, A1);

void setup() {
  Serial.begin(115200);

  mike.begin(8000);

  while (1) {
    Serial.printf("%d\n", mike.read());
    // For stereo/dual mikes, use this line instead
    //   Serial.printf("%d %d\n", mike.read(), mike.read());
  }
}

void loop() {
  /* Nothing here */
}
