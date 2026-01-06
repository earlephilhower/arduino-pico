/*
    pinMode and digitalRead/Write for the Raspberry Pi Pico W RP2040
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

// https://datasheets.raspberrypi.com/rm2/rm2-datasheet.pdf
// https://datasheets.raspberrypi.com/rm2/rm2-product-brief.pdf
// https://pip.raspberrypi.com/categories/1223-design-files
// NOTE: LED1: LED_WLGP0 B1_GPIO0

/*
    Radio Module 2 packages the same Infineon CYW43439 radio used on Raspberry
    Pi Pico W and Pico 2 W, featuring 1×1 single-band 2.4GHz Wi-Fi® 4 (802.11n)
    and Bluetooth® 5.2, with support for both Bluetooth Classic and Bluetooth
    Low Energy. Its integrated internal PA, LNA, and T/R switch deliver
    excellent wireless performance even when sharing a single antenna between
    Wi-Fi and Bluetooth.
 * */

#include "Arduino.h"
#include <cyw43_wrappers.h>

extern "C" void pinMode(pin_size_t pin, PinMode mode) {
    cyw43_pinMode(pin, mode);
}

extern "C" void digitalWrite(pin_size_t pin, PinStatus val) {
    cyw43_digitalWrite(pin, val);
}

extern "C" PinStatus digitalRead(pin_size_t pin) {
    return cyw43_digitalRead(pin);
}
