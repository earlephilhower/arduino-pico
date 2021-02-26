#ifndef Arduino_h
#define Arduino_h

//#include "api/ArduinoAPI.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  LOW     = 0,
  HIGH    = 1,
  CHANGE  = 2,
  FALLING = 3,
  RISING  = 4,
} PinStatus;

typedef enum {
  INPUT           = 0x0,
  OUTPUT          = 0x1,
  INPUT_PULLUP    = 0x2,
  INPUT_PULLDOWN  = 0x3,
} PinMode;

typedef uint8_t pin_size_t;


void pinMode(pin_size_t pinNumber, PinMode pinMode);

// SIO (GPIO)
void digitalWrite(pin_size_t pinNumber, PinStatus status);
PinStatus digitalRead(pin_size_t pinNumber);

// ADC
static const pin_size_t A0 = 26;
static const pin_size_t A1 = 27;
static const pin_size_t A2 = 28;
static const pin_size_t A3 = 29;
int analogRead(pin_size_t pinNumber);

// PWM
void analogWrite(pin_size_t pinNumber, int value);

void delay(unsigned long);
void delayMicroseconds(unsigned int us);
unsigned long millis();


#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
#include "SerialUSB.h"
#endif


// ARM toolchain doesn't provide itoa etc, provide them
#include "api/itoa.h"

#endif // Arduino_h
