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
#include "CoreMutex.h"
#include <hardware/uart.h>
#include <hardware/gpio.h>

// SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The Serialx_available is just a wrapper around Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// HardwareSerial instance if the user doesn't also refer to it.
extern void serialEvent1() __attribute__((weak));
extern void serialEvent2() __attribute__((weak));

bool SerialUART::setRX(pin_size_t rx) {
    constexpr uint32_t valid[2] = { __bitset({1, 13, 17, 29}) /* UART0 */,
                                    __bitset({5, 9, 21, 25})  /* UART1 */};
    if (_running) {
        return false;
    } else if ((1 << rx) & valid[uart_get_index(_uart)]) {
        _rx = rx;
        return true;
    } else {
        return false;
    }
}

bool SerialUART::setTX(pin_size_t tx) {
    constexpr uint32_t valid[2] = { __bitset({0, 12, 16, 28}) /* UART0 */,
                                    __bitset({4, 8, 20, 24})  /* UART1 */};
    if (_running) {
        return false;
    } else if ((1 << tx) & valid[uart_get_index(_uart)]) {
        _tx = tx;
        return true;
    } else {
        return false;
    }
}

SerialUART::SerialUART(uart_inst_t *uart, pin_size_t tx, pin_size_t rx) {
    _uart = uart;
    _tx = tx;
    _rx = rx;
    mutex_init(&_mutex);
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
    if (!_running) {
        return;
    }
    uart_deinit(_uart);
    _running = false;
}

int SerialUART::peek() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return -1;
    }
    if (_peek >= 0) {
        return _peek;
    }
    _peek = uart_getc(_uart);
    return _peek;
}

int SerialUART::read() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
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
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return 0;
    }
    return (uart_is_readable(_uart)) ? 1 : 0;
}

int SerialUART::availableForWrite() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return 0;
    }
    return (uart_is_writable(_uart)) ? 1 : 0;
}

void SerialUART::flush() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return;
    }
    uart_default_tx_wait_blocking();
}

size_t SerialUART::write(uint8_t c) {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return 0;
    }
    uart_putc_raw(_uart, c);
    return 1;
}

size_t SerialUART::write(const uint8_t *p, size_t len) {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
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

void arduino::serialEvent1Run(void) {
    if (serialEvent1 && Serial1.available()) {
      serialEvent1();
    }
}

void arduino::serialEvent2Run(void) {
    if (serialEvent2 && Serial2.available()) {
      serialEvent2();
    }
}
