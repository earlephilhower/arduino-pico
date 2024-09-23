/*
    PWM and analogRead for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include <CoreMutex.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include <hardware/adc.h>

void __clearADCPin(pin_size_t p);

static uint32_t analogScale = 255;
static uint32_t analogFreq = 1000;
static uint64_t pwmInitted = 0;
static bool scaleInitted = false;
static bool adcInitted = false;
static uint16_t analogWritePseudoScale = 1;
static uint16_t analogWriteSlowScale = 1;

auto_init_mutex(_dacMutex);

extern "C" void analogWriteFreq(uint32_t freq) {
    if (freq == analogFreq) {
        return;
    }
    if (freq < 100) {
        DEBUGCORE("ERROR: analogWriteFreq too low (%lu)\n", freq);
        analogFreq = 100;
    } else if (freq > 10'000'000) {
        DEBUGCORE("ERROR: analogWriteFreq too high (%lu)\n", freq);
        analogFreq = 10'000'000;
    } else {
        analogFreq = freq;
    }
    pwmInitted = 0;
    scaleInitted = false;
}

extern "C" void analogWriteRange(uint32_t range) {
    if (range == analogScale) {
        return;
    }
    if ((range >= 3) && (range <= 65535)) {
        analogScale = range;
        pwmInitted = 0;
        scaleInitted = false;
    } else {
        DEBUGCORE("ERROR: analogWriteRange out of range (%lu)\n", range);
    }
}

extern "C" void analogWriteResolution(int res) {
    if ((res >= 2) && (res <= 16)) {
        analogWriteRange((1 << res) - 1);
    } else {
        DEBUGCORE("ERROR: analogWriteResolution out of range (%d)\n", res);
    }
}

extern "C" void analogWrite(pin_size_t pin, int val) {
    CoreMutex m(&_dacMutex);

    if ((pin >= __GPIOCNT) || !m) {
        DEBUGCORE("ERROR: Illegal analogWrite pin (%d)\n", pin);
        return;
    }
    __clearADCPin(pin);
    if (!scaleInitted) {
        // For low frequencies, we need to scale the output max value up to achieve lower periods
        analogWritePseudoScale = 1;
        while (((clock_get_hz(clk_sys) / ((float)analogScale * analogFreq)) > 255.0) && (analogScale < 32678)) {
            analogWritePseudoScale++;
            analogScale *= 2;
            DEBUGCORE("Adjusting analogWrite values PS=%d, scale=%lu\n", analogWritePseudoScale, analogScale);
        }
        // For high frequencies, we need to scale the output max value down to actually hit the frequency target
        analogWriteSlowScale = 1;
        while (((clock_get_hz(clk_sys) / ((float)analogScale * analogFreq)) < 1.0) && (analogScale >= 6)) {
            analogWriteSlowScale++;
            analogScale /= 2;
            DEBUGCORE("Adjusting analogWrite values SS=%d, scale=%lu\n", analogWriteSlowScale, analogScale);
        }
        scaleInitted = true;
    }
    if (!(pwmInitted & (1LL << pwm_gpio_to_slice_num(pin)))) {
        pwm_config c = pwm_get_default_config();
        pwm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / ((float)analogScale * analogFreq));
        pwm_config_set_wrap(&c, analogScale - 1);
        pwm_init(pwm_gpio_to_slice_num(pin), &c, true);
        pwmInitted |= 1LL << pwm_gpio_to_slice_num(pin);
    }

    val <<= analogWritePseudoScale;
    val >>= analogWriteSlowScale;

    if (val < 0) {
        val = 0;
    } else if ((uint32_t)val > analogScale) {
        val = analogScale;
    }

    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(pin, val);
}

auto_init_mutex(_adcMutex);
static uint8_t _readBits = 10;
static uint8_t _lastADCMux = 0;
static uint64_t _adcGPIOInit = 0;

void __clearADCPin(pin_size_t p) {
    _adcGPIOInit &= ~(1LL << p);
}

extern "C" int analogRead(pin_size_t pin) {
    CoreMutex m(&_adcMutex);

    pin_size_t maxPin = __GPIOCNT;
    pin_size_t minPin = __FIRSTANALOGGPIO;

    if ((pin < minPin) || (pin > maxPin) || !m) {
        DEBUGCORE("ERROR: Illegal analogRead pin (%d)\n", pin);
        return 0;
    }
    if (!adcInitted) {
        adc_init();
        adcInitted = true;
    }
    if (!(_adcGPIOInit & (1LL << pin))) {
        adc_gpio_init(pin);
        _adcGPIOInit |= 1LL << pin;
    }
    if (_lastADCMux != pin) {
        adc_select_input(pin - minPin);
        _lastADCMux = pin;
    }
    return (_readBits < 12) ? adc_read() >> (12 - _readBits) : adc_read() << (_readBits - 12);
}

extern "C" float analogReadTemp(float vref) {
    CoreMutex m(&_adcMutex);

    if (!m) {
        return 0.0f; // Deadlock
    }
    if (!adcInitted) {
        adc_init();
        adcInitted = true;
    }
    _lastADCMux = 0;
    adc_set_temp_sensor_enabled(true);
    delay(1); // Allow things to settle.  Without this, readings can be erratic
    adc_select_input(__GPIOCNT - __FIRSTANALOGGPIO); // Temperature sensor
    int v = adc_read();
    adc_set_temp_sensor_enabled(false);
    float t = 27.0f - ((v * vref / 4096.0f) - 0.706f) / 0.001721f; // From the datasheet
    return t;
}

extern "C" void analogReadResolution(int bits) {
    CoreMutex m(&_adcMutex);
    if (m && ((bits > 0) && (bits < 32))) {
        _readBits = bits;
    }
}
