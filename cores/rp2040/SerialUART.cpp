/*
    Serial-over-UART for the Raspberry Pi Pico RP2040

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

bool SerialUART::setRX(pin_size_t pin) {
    constexpr uint32_t valid[2] = { __bitset({1, 13, 17, 29}) /* UART0 */,
                                    __bitset({5, 9, 21, 25})  /* UART1 */
                                  };
    if ((!_running) && ((1 << pin) & valid[uart_get_index(_uart)])) {
        _rx = pin;
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set Serial%d.RX while running", uart_get_index(_uart) + 1);
    } else {
        panic("FATAL: Attempting to set Serial%d.RX to illegal pin %d", uart_get_index(_uart) + 1, pin);
    }
    return false;
}

bool SerialUART::setTX(pin_size_t pin) {
    constexpr uint32_t valid[2] = { __bitset({0, 12, 16, 28}) /* UART0 */,
                                    __bitset({4, 8, 20, 24})  /* UART1 */
                                  };
    if ((!_running) && ((1 << pin) & valid[uart_get_index(_uart)])) {
        _tx = pin;
        return true;
    }

    if (_running) {
        panic("FATAL: Attempting to set Serial%d.TX while running", uart_get_index(_uart) + 1);
    } else {
        panic("FATAL: Attempting to set Serial%d.TX to illegal pin %d", uart_get_index(_uart) + 1, pin);
    }
    return false;
}

SerialUART::SerialUART(uart_inst_t *uart, pin_size_t tx, pin_size_t rx) {
    _uart = uart;
    _tx = tx;
    _rx = rx;
    mutex_init(&_mutex);
}

static void _uart0IRQ();
static void _uart1IRQ();

void SerialUART::begin(unsigned long baud, uint16_t config) {
    _baud = baud;
    uart_init(_uart, baud);
    int bits, stop;
    uart_parity_t parity;
    switch (config & SERIAL_PARITY_MASK) {
    case SERIAL_PARITY_EVEN:
        parity = UART_PARITY_EVEN;
        break;
    case SERIAL_PARITY_ODD:
        parity = UART_PARITY_ODD;
        break;
    default:
        parity = UART_PARITY_NONE;
        break;
    }
    switch (config & SERIAL_STOP_BIT_MASK) {
    case SERIAL_STOP_BIT_1:
        stop = 1;
        break;
    default:
        stop = 2;
        break;
    }
    switch (config & SERIAL_DATA_MASK) {
    case SERIAL_DATA_5:
        bits = 5;
        break;
    case SERIAL_DATA_6:
        bits = 6;
        break;
    case SERIAL_DATA_7:
        bits = 7;
        break;
    default:
        bits = 8;
        break;
    }
    uart_set_format(_uart, bits, stop, parity);
    gpio_set_function(_tx, GPIO_FUNC_UART);
    gpio_set_function(_rx, GPIO_FUNC_UART);
    _writer = 0;
    _reader = 0;

    if (_uart == uart0) {
        irq_set_exclusive_handler(UART0_IRQ, _uart0IRQ);
        irq_set_enabled(UART0_IRQ, true);
    } else {
        irq_set_exclusive_handler(UART1_IRQ, _uart1IRQ);
        irq_set_enabled(UART1_IRQ, true);
    }
    uart_get_hw(_uart)->imsc |= UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS;
    _running = true;
}

void SerialUART::end() {
    if (!_running) {
        return;
    }
    if (_uart == uart0) {
        irq_set_enabled(UART0_IRQ, false);
    } else {
        irq_set_enabled(UART1_IRQ, false);
    }
    uart_deinit(_uart);
    _running = false;
}

int SerialUART::peek() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return -1;
    }
    uint32_t start = millis();
    uint32_t now = millis();
    while ((now - start) < _timeout) {
        if (_writer != _reader) {
            return _queue[_reader];
        }
        delay(1);
        now = millis();
    }
    return -1; // Nothing available before timeout
}

int SerialUART::read() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return -1;
    }
    uint32_t start = millis();
    uint32_t now = millis();
    while ((now - start) < _timeout) {
        if (_writer != _reader) {
            auto ret = _queue[_reader];
            _reader = (_reader + 1) % sizeof(_queue);
            return ret;
        }
        delay(1);
        now = millis();
    }
    return -1; // Timeout
}

int SerialUART::available() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return 0;
    }
    return (_writer - _reader) % sizeof(_queue);
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
    uart_tx_wait_blocking(_uart);
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

SerialUART Serial1(uart0, PIN_SERIAL1_TX, PIN_SERIAL1_RX);
SerialUART Serial2(uart1, PIN_SERIAL2_TX, PIN_SERIAL2_RX);

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

// IRQ handler, called when FIFO > 1/4 full or when it had held unread data for >32 bit times
void __not_in_flash_func(SerialUART::_handleIRQ)() {
    uart_get_hw(_uart)->icr |= UART_UARTICR_RTIC_BITS | UART_UARTICR_RXIC_BITS;
    while ((uart_is_readable(_uart)) && ((_writer + 1) % sizeof(_queue) != _reader)) {
        _queue[_writer] = uart_getc(_uart);
        asm volatile("" ::: "memory"); // Ensure the queue is written before the written count advances
        _writer = (_writer + 1) % sizeof(_queue);
    }
}

static void __not_in_flash_func(_uart0IRQ)() {
    Serial1._handleIRQ();
}

static void __not_in_flash_func(_uart1IRQ)() {
    Serial2._handleIRQ();
}
