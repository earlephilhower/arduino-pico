/* Taken from https://github.com/arduino/ArduinoCore-mbed/tree/master/variants/NANO_RP2040_CONNECT */

#ifndef _NINA_PINS_
#define _NINA_PINS_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "Arduino.h"

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

enum NinaPin {
  LEDR = 27,
  LEDG = 25,
  LEDB = 26//,
  //A4   = 34,
  //A5   = 39,
  //A6   = 36,
  //A7   = 35
};

/******************************************************************************
 * FUNCTION DECLARATION
 ******************************************************************************/

void      pinMode     (NinaPin pin, PinMode mode);
PinStatus digitalRead (NinaPin pin);
void      digitalWrite(NinaPin pin, PinStatus value);
int       analogRead  (NinaPin pin);
void      analogWrite (NinaPin pin, int value);

#endif /* _NINA_PINS_ */
