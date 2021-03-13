/*
 * Serial-over-UART for the Raspberry Pi Pico RP2040
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

#include "SerialUART.h"
#include <hardware/gpio.h>

bool SerialUART::setPinout(pin_size_t tx, pin_size_t rx) {
    const uint32_t uart_tx[2] = { 0b000010001000000000001000100000, 0b100000000000100010000000000010 };
    const uint32_t uart_rx[2] = { 0b000001000100000000000100010000, 0b010000000000010001000000000001 };
    if ( ((1 << tx) & uart_tx[uart_get_index(_uart)]) &&
        ((1 << rx) & uart_rx[uart_get_index(_uart)]) ) {
        if (_running) {
            pinMode(_tx, INPUT);
            pinMode(_rx, INPUT);
        }
        _tx = tx;
        _rx = rx;
        if (_running) {
            gpio_set_function(_tx, GPIO_FUNC_UART);
            gpio_set_function(_rx, GPIO_FUNC_UART);
        }
        return true;
    }
    return false;
}


void SerialUART::begin(unsigned long baud, uint16_t config) {
    _baud = baud;
    uart_init(_uart, baud);
    int bits, stop;
    uart_parity_t parity;
    switch (config & SERIAL_PARITY_MASK) {
        case SERIAL_PARITY_EVEN: parity = UART_PARITY_EVEN; break;
        case SERIAL_PARITY_ODD: parity = UART_PARITY_ODD; break;
        default: parity = UART_PARITY_NONE; break;
    }
    switch ( config & SERIAL_STOP_BIT_MASK) {
        case SERIAL_STOP_BIT_1: stop = 1; break;
        default: stop = 2; break;
    }
    switch (config & SERIAL_DATA_MASK) {
        case SERIAL_DATA_5: bits = 5; break;
        case SERIAL_DATA_6: bits = 6; break;
        case SERIAL_DATA_7: bits = 7; break;
        default: bits = 8; break;
    }
    uart_set_format(_uart, bits, stop, parity);
    gpio_set_function(_tx, GPIO_FUNC_UART);
    gpio_set_function(_rx, GPIO_FUNC_UART);
    _running = true;
    _peek = -1;
}

void SerialUART::end() {
    uart_deinit(_uart);
    _running = false;
}

int SerialUART::peek() {
    if (!_running) {
        return -1;
    }
    if (_peek >= 0) {
        return _peek;
    }
    _peek = uart_getc(_uart);
    return _peek;
}

int SerialUART::read() {
    if (!_running) {
        return -1;
    }
    if (_peek >= 0) {
        int ret = _peek;
        _peek = -1;
        return ret;
    }
    return uart_getc(_uart);
}

int SerialUART::available() {
    if (!_running) {
        return 0;
    }
    return (uart_is_readable(_uart)) ? 1 : 0;
}

int SerialUART::availableForWrite() {
    if (!_running) {
        return 0;
    }
    return (uart_is_writable(_uart)) ? 1 : 0;
}

void SerialUART::flush() {
    // TODO, must be smarter way.  Now, just sleep long enough to guarantee a full FIFO goes out (very conservative)
    //int us_per_bit = 1 + (1000000 / _baud);
    //delayMicroseconds(us_per_bit * 32 * 8);
    uart_default_tx_wait_blocking();
}

size_t SerialUART::write(uint8_t c) {
    if (!_running) {
        return 0;
    }
    uart_putc_raw(_uart, c);
    return 1;
}

size_t SerialUART::write(const uint8_t *p, size_t len) {
    if (!_running) {
        return 0;
    }
    size_t cnt = len;
    while (cnt) {
        uart_putc_raw(_uart, *p);
        cnt--;
        p++;
    }
    return len;
}

SerialUART::operator bool() {
    return _running;
}

SerialUART Serial1(uart0, 0, 1);
SerialUART Serial2(uart1, 4, 5);

