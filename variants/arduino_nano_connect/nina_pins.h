/* Taken from https://github.com/arduino/ArduinoCore-mbed/tree/master/variants/NANO_RP2040_CONNECT */

#ifndef _NINA_PINS_
#define _NINA_PINS_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "Arduino.h"
//#include "WiFiNINA.h"
/******************************************************************************
 * PREPROCESSOR-MAGIC
 ******************************************************************************/

#if __has_include("WiFiNINA.h")
#  define NINA_ATTRIBUTE
#else
#  define NINA_ATTRIBUTE __attribute__ ((error("Please include WiFiNINA.h to use this pin")))
#endif

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

void      NINA_ATTRIBUTE pinMode     (NinaPin pin, PinMode mode);
PinStatus NINA_ATTRIBUTE digitalRead (NinaPin pin);
void      NINA_ATTRIBUTE digitalWrite(NinaPin pin, PinStatus value);
int       NINA_ATTRIBUTE analogRead  (NinaPin pin);
void      NINA_ATTRIBUTE analogWrite (NinaPin pin, int value);

#undef NINA_ATTRIBUTE

#endif /* _NINA_PINS_ */
