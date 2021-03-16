/*
 * PIO-based Servo class for Rasperry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 * Original Copyright (c) 2015 Michael C. Miller. All right reserved.
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

#include <Arduino.h>
#include <Servo.h>
#include <hardware/clocks.h>
#include <hardware/pio.h>
#include "pwm.pio.h"

// similiar to map but will have increased accuracy that provides a more
// symmetrical api (call it and use result to reverse will provide the original value)
int improved_map(int value, int minIn, int maxIn, int minOut, int maxOut)
{
    const int rangeIn = maxIn - minIn;
    const int rangeOut = maxOut - minOut;
    const int deltaIn = value - minIn;
    // fixed point math constants to improve accurancy of divide and rounding
    constexpr int fixedHalfDecimal = 1;
    constexpr int fixedDecimal = fixedHalfDecimal * 2;

    return ((deltaIn * rangeOut * fixedDecimal) / (rangeIn) + fixedHalfDecimal) / fixedDecimal + minOut;
}

//-------------------------------------------------------------------
// Servo class methods

Servo::Servo()
{
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

static bool _findFreeSM(PIO *pio, int *smIdx) {
    int idx = pio_claim_unused_sm(pio0, false);
    if (idx >= 0) {
        *pio = pio0;
        *smIdx = idx;
        return true;
    }
    idx = pio_claim_unused_sm(pio1, false);
    if (idx >= 0) {
        *pio = pio1;
        *smIdx = idx;
        return true;
    }
    return false;
}


static int _usToPIO(int us) {
    return (us * ( clock_get_hz(clk_sys) / 1000000 )) / 3;
}



// Write `period` to the input shift register
static void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

// Write `level` to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
    pio_sm_put_blocking(pio, sm, level);
}

int Servo::attach(pin_size_t pin)
{
    return attach(pin, DEFAULT_MIN_PULSE_WIDTH, DEFAULT_MAX_PULSE_WIDTH);
}

int Servo::attach(pin_size_t pin, int minUs, int maxUs)
{
    return attach(pin, minUs, maxUs, _valueUs);
}

int Servo::attach(pin_size_t pin, int minUs, int maxUs, int value)
{
    // keep the min and max within 200-3000 us, these are extreme
    // ranges and should support extreme servos while maintaining
    // reasonable ranges
    _maxUs = max(250, min(3000, maxUs));
    _minUs = max(200, min(_maxUs, minUs));

    if (!_attached) {
        if (!_findFreeSM(&_pio, &_smIdx)) {
            // ERROR, no free slots
            return -1;
        }
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
        // Load the PIO program - TODO , only one copy needed for all SMs!
        _pgmOffset = pio_add_program(_pio, &pwm_program);
        pwm_program_init(_pio, _smIdx, _pgmOffset, pin);
        pio_pwm_set_period(_pio, _smIdx, _usToPIO(REFRESH_INTERVAL) );
        _pin = pin;
	_attached = true;
    }

    write(value);

    return pin;
}

void Servo::detach()
{
    if (_attached) {
        pio_sm_set_enabled(_pio, _smIdx, false);
        pio_remove_program(_pio, &pwm_program, _pgmOffset);
        pio_sm_unclaim(_pio, _smIdx);
        _attached = false;
        _valueUs = DEFAULT_NEUTRAL_PULSE_WIDTH;
    }
}

void Servo::write(int value)
{
    // treat any value less than 200 as angle in degrees (values equal or larger are handled as microseconds)
    if (value < 200) {
        // assumed to be 0-180 degrees servo
        value = constrain(value, 0, 180);
        value = improved_map(value, 0, 180, _minUs, _maxUs);
    }
    writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value)
{
    value = constrain(value, _minUs, _maxUs);
    _valueUs = value;
    if (_attached) {
        pio_pwm_set_level(_pio, _smIdx, _usToPIO(value));
    }
}

int Servo::read() // return the value as degrees
{
    // read returns the angle for an assumed 0-180
    return improved_map(readMicroseconds(), _minUs, _maxUs, 0, 180);
}

int Servo::readMicroseconds()
{
    return _valueUs;
}

bool Servo::attached()
{
    return _attached;
}
