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

int getAnalogReadResolution();

class NinaPin {
public:
	NinaPin(int _pin) : pin(_pin) {};
	int get() {
		return pin;
	};
	int analogReadResolution() {
		return getAnalogReadResolution();
	};
	bool operator== (NinaPin const & other) const {
		return pin == other.pin;
	}
	//operator int() = delete;
	__attribute__ ((error("Change me to a #define"))) operator int();
private:
	int pin;
};

extern NinaPin  LEDR;
extern NinaPin  LEDG;
extern NinaPin  LEDB;
extern NinaPin  A4;
extern NinaPin  A5;
extern NinaPin  A6;
extern NinaPin  A7;

#define NINA_PINS_AS_CLASS

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
