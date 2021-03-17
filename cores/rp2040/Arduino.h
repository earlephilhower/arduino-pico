/*
 * Arduino header for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "api/ArduinoAPI.h"

#include <pins_arduino.h>

#include "debug_internal.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

void pinMode(pin_size_t pinNumber, PinMode pinMode);

// SIO (GPIO)
void digitalWrite(pin_size_t pinNumber, PinStatus status);
PinStatus digitalRead(pin_size_t pinNumber);

// ADC
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
#include "SerialUART.h"
#include "RP2040.h"
#endif


// ARM toolchain doesn't provide itoa etc, provide them
#include "api/itoa.h"

#endif // Arduino_h
