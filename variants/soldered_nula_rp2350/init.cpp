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

#include <cyw43_wrappers.h>
#include "pico/cyw43_driver.h"

extern "C" void initVariant() {
    static uint cyw43_pin_array[CYW43_PIN_INDEX_WL_COUNT] = {24, 38, 38, 38, 37, 36};
    cyw43_set_pins_wl(cyw43_pin_array);
    init_cyw43_wifi();
}
