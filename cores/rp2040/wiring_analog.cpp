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

static uint32_t analogScale = 255;
static uint32_t analogFreq = 1000;
static bool pwmInitted = false;
static bool adcInitted = false;
static uint16_t analogWritePseudoScale = 1;
static uint16_t analogWriteSlowScale = 1;

auto_init_mutex(_dacMutex);

extern "C" void analogWriteFreq(uint32_t freq) {
    if (freq == analogFreq) {
        return;
    }
    if (freq < 100) {
        DEBUGCORE("ERROR: analogWriteFreq too low (%d)\n", freq);
        analogFreq = 100;
    } else if (freq > 1000000) {
        DEBUGCORE("ERROR: analogWriteFreq too high (%d)\n", freq);
        analogFreq = 1000000;
    } else {
        analogFreq = freq;
    }
    pwmInitted = false;
}

extern "C" void analogWriteRange(uint32_t range) {
    if (range == analogScale) {
        return;
    }
    if ((range >= 15) && (range <= 65535)) {
        analogScale = range;
        pwmInitted = false;
    } else {
        DEBUGCORE("ERROR: analogWriteRange out of range (%d)\n", range);
    }
}

extern "C" void analogWriteResolution(int res) {
    if ((res >= 4) && (res <= 16)) {
        analogWriteRange((1 << res) - 1);
    } else {
        DEBUGCORE("ERROR: analogWriteResolution out of range (%d)\n", res);
    }
}

extern "C" void analogWrite(pin_size_t pin, int val) {
    CoreMutex m(&_dacMutex);

    if ((pin > 29) || !m) {
        DEBUGCORE("ERROR: Illegal analogWrite pin (%d)\n", pin);
        return;
    }
    if (!pwmInitted) {
        // For low frequencies, we need to scale the output max value up to achieve lower periods
        analogWritePseudoScale = 1;
        while (((clock_get_hz(clk_sys) / (float)(analogScale * analogFreq)) > 255.0) && (analogScale < 32678)) {
            analogWritePseudoScale++;
            analogScale *= 2;
            DEBUGCORE("Adjusting analogWrite values PS=%d, scale=%d\n", analogWritePseudoScale, analogScale);
        }
        // For high frequencies, we need to scale the output max value down to actually hit the frequency target
        analogWriteSlowScale = 1;
        while (((clock_get_hz(clk_sys) / (float)(analogScale * analogFreq)) < 2.0) && (analogScale > 32)) {
            analogWriteSlowScale++;
            analogScale /= 2;
            DEBUGCORE("Adjusting analogWrite values SS=%d, scale=%d\n", analogWriteSlowScale, analogScale);
        }

        pwm_config c = pwm_get_default_config();
        pwm_config_set_clkdiv(&c, clock_get_hz(clk_sys) / (float)(analogScale * analogFreq));
        pwm_config_set_wrap(&c, analogScale);
        for (int i = 0; i < 30; i++) {
            pwm_init(pwm_gpio_to_slice_num(i), &c, true);
        }
        pwmInitted = true;
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
static int _readBits = 10;

extern "C" int analogRead(pin_size_t pin) {
    CoreMutex m(&_adcMutex);

    pin_size_t maxPin = max(A0, A3);
    pin_size_t minPin = min(A0, A3);

    if ((pin < minPin) || (pin > maxPin) || !m) {
        DEBUGCORE("ERROR: Illegal analogRead pin (%d)\n", pin);
        return 0;
    }
    if (!adcInitted) {
        adc_init();
    }
    adc_gpio_init(pin);
    adc_select_input(pin - minPin);
    return (_readBits < 12) ? adc_read() >> (12 - _readBits) : adc_read() << (_readBits - 12);
}

extern "C" float analogReadTemp() {
    CoreMutex m(&_adcMutex);

    if (!m) {
        return 0.0f; // Deadlock
    }
    if (!adcInitted) {
        adc_init();
    }
    adc_set_temp_sensor_enabled(true);
    delay(1); // Allow things to settle.  Without this, readings can be erratic
    adc_select_input(4); // Temperature sensor
    int v = adc_read();
    adc_set_temp_sensor_enabled(false);
    float t = 27.0f - ((v * 3.3f / 4096.0f) - 0.706f) / 0.001721f; // From the datasheet
    return t;
}

extern "C" void analogReadResolution(int bits) {
    CoreMutex m(&_adcMutex);
    if (m && ((bits > 0) && (bits < 32))) {
        _readBits = bits;
    }
}
