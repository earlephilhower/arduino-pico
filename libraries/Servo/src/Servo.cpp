/*
    PIO-based Servo class for Rasperry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
    Original Copyright (c) 2015 Michael C. Miller. All right reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <Servo.h>
#include <hardware/clocks.h>
#include <hardware/pio.h>

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

#include "servo.pio.h"
static PIOProgram _servoPgm(&servo_program);

// Similar to map but will have increased accuracy that provides a more
// symmetrical api (call it and use result to reverse will provide the original value)
int improved_map(int value, int minIn, int maxIn, int minOut, int maxOut) {
    const int rangeIn = maxIn - minIn;
    const int rangeOut = maxOut - minOut;
    const int deltaIn = value - minIn;
    // fixed point math constants to improve accuracy of divide and rounding
    constexpr int fixedHalfDecimal = 1;
    constexpr int fixedDecimal = fixedHalfDecimal * 2;

    return ((deltaIn * rangeOut * fixedDecimal) / (rangeIn) + fixedHalfDecimal) / fixedDecimal + minOut;
}

//-------------------------------------------------------------------
// Servo class methods

Servo::Servo() {
    _attached = false;
    _valueUs = DEFAULT_NEUTRAL_PULSE_WIDTH;
    _minUs = DEFAULT_MIN_PULSE_WIDTH;
    _maxUs = DEFAULT_MAX_PULSE_WIDTH;
    _pio = nullptr;
    _smIdx = -1;
    _pgmOffset = -1;
}

Servo::~Servo() {
    detach();
}

int Servo::attach(pin_size_t pin) {
    return attach(pin, DEFAULT_MIN_PULSE_WIDTH, DEFAULT_MAX_PULSE_WIDTH);
}

int Servo::attach(pin_size_t pin, int minUs, int maxUs) {
    return attach(pin, minUs, maxUs, _valueUs);
}

int Servo::attach(pin_size_t pin, int minUs, int maxUs, int value) {
    // keep the min and max within 200-3000 us, these are extreme
    // ranges and should support extreme servos while maintaining
    // reasonable ranges
    _maxUs = max(250, min(3000, maxUs));
    _minUs = max(200, min(_maxUs, minUs));

    if (!_attached) {
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
        _pin = pin;
        if (!_servoPgm.prepare(&_pio, &_smIdx, &_pgmOffset, pin, 1)) {
            // ERROR, no free slots
            return -1;
        }
        _attached = true;
        servo_program_init(_pio, _smIdx, _pgmOffset, pin);
        pio_sm_set_enabled(_pio, _smIdx, false);
        pio_sm_put_blocking(_pio, _smIdx, RP2040::usToPIOCycles(REFRESH_INTERVAL) / 3);
        pio_sm_exec(_pio, _smIdx, pio_encode_pull(false, false));
        pio_sm_exec(_pio, _smIdx, pio_encode_out(pio_isr, 32));
        write(value);
        pio_sm_exec(_pio, _smIdx, pio_encode_pull(false, false));
        pio_sm_exec(_pio, _smIdx, pio_encode_mov(pio_x, pio_osr));
        pio_sm_set_enabled(_pio, _smIdx, true);
    }

    write(value);

    return pin;
}

void Servo::detach() {
    if (_attached) {
        // Set a 0 for the width and then wait for the halt loop
        pio_sm_put_blocking(_pio, _smIdx, 0);
        delayMicroseconds(5);  // Avoid race condition
        do {
            // Do nothing until we are stuck in the halt loop (avoid short pulses
        } while (pio_sm_get_pc(_pio, _smIdx) != servo_offset_halt + _pgmOffset);
        pio_sm_set_enabled(_pio, _smIdx, false);
        pio_sm_unclaim(_pio, _smIdx);
        _attached = false;
        _valueUs = DEFAULT_NEUTRAL_PULSE_WIDTH;
    }
}

void Servo::write(int value) {
    // treat any value less than 200 as angle in degrees (values equal or larger are handled as microseconds)
    if (value < 200) {
        // assumed to be 0-180 degrees servo
        value = constrain(value, 0, 180);
        value = improved_map(value, 0, 180, _minUs, _maxUs);
    }
    writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value) {
    value = constrain(value, _minUs, _maxUs);
    _valueUs = value;
    if (_attached) {
        pio_sm_clear_fifos(_pio, _smIdx); // Remove any old updates that haven't yet taken effect
        pio_sm_put_blocking(_pio, _smIdx, RP2040::usToPIOCycles(value) / 3);
    }
}

int Servo::read() { // return the value as degrees
    // read returns the angle for an assumed 0-180
    return improved_map(readMicroseconds(), _minUs, _maxUs, 0, 180);
}

int Servo::readMicroseconds() {
    return _valueUs;
}

bool Servo::attached() {
    return _attached;
}
