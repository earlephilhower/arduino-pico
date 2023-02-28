/*
    Initialize the Pico W WiFi driver

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include <pico/cyw43_arch.h>
#include <pico/lwip_nosys.h>
#include "hardware/resets.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#ifndef WIFICC
#define WIFICC CYW43_COUNTRY_WORLDWIDE
#endif

// Taken from https://datasheets.raspberrypi.com/picow/connecting-to-the-internet-with-pico-w.pdf
// also discussion in https://github.com/earlephilhower/arduino-pico/issues/849
static bool CheckPicoW() {
    adc_init();
    auto dir = gpio_get_dir(29);
    auto fnc = gpio_get_function(29);
    adc_gpio_init(29);
    adc_select_input(3);
    auto adc29 = adc_read();
    gpio_set_function(29, fnc);
    gpio_set_dir(29, dir);

    dir = gpio_get_dir(25);
    fnc = gpio_get_function(25);
    gpio_init(25);
    gpio_set_dir(25, GPIO_IN);
    auto gp25 = gpio_get(25);
    gpio_set_function(25, fnc);
    gpio_set_dir(25, dir);

    if (gp25) {
        return true; // Can't tell, so assume yes
    } else if (adc29 < 200) {
        return true; // PicoW
    } else {
        return false;
    }
}

bool __isPicoW = true;

extern "C" void initVariant() {
    __isPicoW = CheckPicoW();
    if (__isPicoW) {
        cyw43_arch_init_with_country(WIFICC);
    }
}
