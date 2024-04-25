/*
    wiring_shift.c - shiftOut() function
    Part of Arduino - http://www.arduino.cc/

    Copyright (c) 2005-2006 David A. Mellis

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General
    Public License along with this library; if not, write to the
    Free Software Foundation, Inc., 59 Temple Place, Suite 330,
    Boston, MA  02111-1307  USA
*/

#include <Arduino.h>

extern "C" uint8_t shiftIn(pin_size_t dataPin, pin_size_t clockPin, BitOrder bitOrder) {
    uint8_t value = 0;
    uint8_t i;
    if (dataPin > 29) {
        DEBUGCORE("ERROR: Illegal dataPin in shiftIn (%d)\n", dataPin);
        return 0;
    }
    if (clockPin > 29) {
        DEBUGCORE("ERROR: Illegal clockPin in shiftIn (%d)\n", clockPin);
        return 0;
    }
    for (i = 0; i < 8; ++i) {
        digitalWrite(clockPin, HIGH);
        if (bitOrder == LSBFIRST) {
            value |= digitalRead(dataPin) << i;
        } else {
            value |= digitalRead(dataPin) << (7 - i);
        }
        digitalWrite(clockPin, LOW);
    }
    return value;
}

extern "C" void shiftOut(pin_size_t dataPin, pin_size_t clockPin, BitOrder bitOrder, uint8_t val) {
    uint8_t i;
    if (dataPin > 29) {
        DEBUGCORE("ERROR: Illegal dataPin in shiftOut (%d)\n", dataPin);
        return;
    }
    if (clockPin > 29) {
        DEBUGCORE("ERROR: Illegal clockPin in shiftOut (%d)\n", clockPin);
        return;
    }
    for (i = 0; i < 8; i++)  {
        if (bitOrder == LSBFIRST) {
            digitalWrite(dataPin, !!(val & (1 << i)) ? HIGH : LOW);
        } else {
            digitalWrite(dataPin, !!(val & (1 << (7 - i))) ? HIGH : LOW);
        }

        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}
