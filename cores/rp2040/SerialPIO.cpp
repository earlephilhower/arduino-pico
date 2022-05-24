/*
    Serial-over-PIO for the Raspberry Pi Pico RP2040

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

#include "SerialPIO.h"
#include "CoreMutex.h"
#include <hardware/gpio.h>
#include <map>
#include "pio_uart.pio.h"


// ------------------------------------------------------------------------
// -- Generates a unique program for differing bit lengths
static std::map<int, PIOProgram*> _txMap;
static std::map<int, PIOProgram*> _rxMap;

// Duplicate a program and replace the first insn with a "set x, repl"
static pio_program_t *pio_make_uart_prog(int repl, const pio_program_t *pg) {
    pio_program_t *p = new pio_program_t;
    p->length = pg->length;
    p->origin = pg->origin;
    uint16_t *insn = (uint16_t *)malloc(p->length * 2);
    if (!insn) {
        delete p;
        return nullptr;
    }
    memcpy(insn, pg->instructions, p->length * 2);
    insn[0] = pio_encode_set(pio_x, repl);
    p->instructions = insn;
    return p;
}

static PIOProgram *_getTxProgram(int bits) {
    auto f = _txMap.find(bits);
    if (f == _txMap.end()) {
        pio_program_t * p = pio_make_uart_prog(bits, &pio_tx_program);
        _txMap.insert({bits, new PIOProgram(p)});
        f = _txMap.find(bits);
    }
    return f->second;
}

static PIOProgram *_getRxProgram(int bits) {
    auto f = _rxMap.find(bits);
    if (f == _rxMap.end()) {
        pio_program_t * p = pio_make_uart_prog(bits, &pio_rx_program);
        _rxMap.insert({bits, new PIOProgram(p)});
        f = _rxMap.find(bits);
    }
    return f->second;
}
// ------------------------------------------------------------------------

// TODO - this works, but there must be a faster/better way...
static int _parity(int bits, int data) {
    int p = 0;
    for (int b = 0; b < bits; b++) {
        p ^= (data & (1 << b)) ? 1 : 0;
    }
    return p;
}

// We need to cache generated SerialPIOs so we can add data to them from
// the shared handler
static SerialPIO *_pioSP[2][4];
static void __not_in_flash_func(_fifoIRQ)() {
    for (int p = 0; p < 2; p++) {
        for (int sm = 0; sm < 4; sm++) {
            SerialPIO *s = _pioSP[p][sm];
            if (s) {
                s->_handleIRQ();
                pio_interrupt_clear((p == 0) ? pio0 : pio1, sm);
            }
        }
    }
}

void __not_in_flash_func(SerialPIO::_handleIRQ)() {
    if (_rx == NOPIN) {
        return;
    }
    while (!pio_sm_is_rx_fifo_empty(_rxPIO, _rxSM)) {
        uint32_t decode = _rxPIO->rxf[_rxSM];
        decode >>= 33 - _rxBits;
        uint32_t val = 0;
        for (int b = 0; b < _bits + 1; b++) {
            val |= (decode & (1 << (b * 2))) ? 1 << b : 0;
        }
        if (_parity == UART_PARITY_EVEN) {
            int p = ::_parity(_bits, val);
            int r = (val & (1 << _bits)) ? 1 : 0;
            if (p != r) {
                // TODO - parity error
                continue;
            }
        } else if (_parity == UART_PARITY_ODD) {
            int p = ::_parity(_bits, val);
            int r = (val & (1 << _bits)) ? 1 : 0;
            if (p == r) {
                // TODO - parity error
                continue;
            }
        }

        auto next_writer = _writer + 1;
        if (next_writer == _fifoSize) {
            next_writer = 0;
        }
        if (next_writer != _reader) {
            _queue[_writer] = val & ((1 << _bits) -  1);
            asm volatile("" ::: "memory"); // Ensure the queue is written before the written count advances
            _writer = next_writer;
        } else {
            _overflow = true;
        }
    }
}

SerialPIO::SerialPIO(pin_size_t tx, pin_size_t rx, size_t fifoSize) {
    _tx = tx;
    _rx = rx;
    _fifoSize = fifoSize + 1; // Always one unused entry
    _queue = new uint8_t[_fifoSize];
    mutex_init(&_mutex);
}

SerialPIO::~SerialPIO() {
    end();
    delete[] _queue;
}

void SerialPIO::begin(unsigned long baud, uint16_t config) {
    _overflow = false;
    _baud = baud;
    switch (config & SERIAL_PARITY_MASK) {
    case SERIAL_PARITY_EVEN:
        _parity = UART_PARITY_EVEN;
        break;
    case SERIAL_PARITY_ODD:
        _parity = UART_PARITY_ODD;
        break;
    default:
        _parity = UART_PARITY_NONE;
        break;
    }
    switch (config & SERIAL_STOP_BIT_MASK) {
    case SERIAL_STOP_BIT_1:
        _stop = 1;
        break;
    default:
        _stop = 2;
        break;
    }
    switch (config & SERIAL_DATA_MASK) {
    case SERIAL_DATA_5:
        _bits = 5;
        break;
    case SERIAL_DATA_6:
        _bits = 6;
        break;
    case SERIAL_DATA_7:
        _bits = 7;
        break;
    default:
        _bits = 8;
        break;
    }

    if ((_tx == NOPIN) && (_rx == NOPIN)) {
        DEBUGCORE("ERROR: No pins specified for SerialPIO\n");
        return;
    }

    if (_tx != NOPIN) {
        _txBits = _bits + _stop + (_parity != UART_PARITY_NONE ? 1 : 0) + 1/*start bit*/;
        _txPgm = _getTxProgram(_txBits);
        int off;
        if (!_txPgm->prepare(&_txPIO, &_txSM, &off)) {
            DEBUGCORE("ERROR: Unable to allocate PIO TX UART, out of PIO resources\n");
            // ERROR, no free slots
            return;
        }

        digitalWrite(_tx, HIGH);
        pinMode(_tx, OUTPUT);

        pio_tx_program_init(_txPIO, _txSM, off, _tx);
        pio_sm_clear_fifos(_txPIO, _txSM); // Remove any existing data

        // Put the divider into ISR w/o using up program space
        pio_sm_put_blocking(_txPIO, _txSM, clock_get_hz(clk_sys) / _baud - 2);
        pio_sm_exec(_txPIO, _txSM, pio_encode_pull(false, false));
        pio_sm_exec(_txPIO, _txSM, pio_encode_mov(pio_isr, pio_osr));

        // Start running!
        pio_sm_set_enabled(_txPIO, _txSM, true);
    }
    if (_rx != NOPIN) {
        _writer = 0;
        _reader = 0;

        _rxBits = 2 * (_bits + _stop + (_parity != UART_PARITY_NONE ? 1 : 0) + 1) - 1;
        _rxPgm = _getRxProgram(_rxBits);
        int off;
        if (!_rxPgm->prepare(&_rxPIO, &_rxSM, &off)) {
            DEBUGCORE("ERROR: Unable to allocate PIO RX UART, out of PIO resources\n");
            return;
        }
        // Stash away the created RX port for the IRQ handler
        _pioSP[pio_get_index(_rxPIO)][_rxSM] = this;

        pinMode(_rx, INPUT);
        pio_rx_program_init(_rxPIO, _rxSM, off, _rx);
        pio_sm_clear_fifos(_rxPIO, _rxSM); // Remove any existing data

        // Put phase divider into OSR w/o using add'l program memory
        pio_sm_put_blocking(_rxPIO, _rxSM, clock_get_hz(clk_sys) / (_baud * 2) - 5 /* insns in PIO halfbit loop */);
        pio_sm_exec(_rxPIO, _rxSM, pio_encode_pull(false, false));

        // Join the TX FIFO to the RX one now that we don't need it
        _rxPIO->sm[_rxSM].shiftctrl |= 0x80000000;

        // Enable interrupts on rxfifo
        switch (_rxSM) {
        case 0: pio_set_irq0_source_enabled(_rxPIO, pis_sm0_rx_fifo_not_empty, true); break;
        case 1: pio_set_irq0_source_enabled(_rxPIO, pis_sm1_rx_fifo_not_empty, true); break;
        case 2: pio_set_irq0_source_enabled(_rxPIO, pis_sm2_rx_fifo_not_empty, true); break;
        case 3: pio_set_irq0_source_enabled(_rxPIO, pis_sm3_rx_fifo_not_empty, true); break;
        }
        auto irqno = pio_get_index(_rxPIO) == 0 ? PIO0_IRQ_0 : PIO1_IRQ_0;
        irq_set_exclusive_handler(irqno, _fifoIRQ);
        irq_set_enabled(irqno, true);

        pio_sm_set_enabled(_rxPIO, _rxSM, true);
    }

    _running = true;
}

void SerialPIO::end() {
    if (!_running) {
        return;
    }
    if (_tx != NOPIN) {
        pio_sm_set_enabled(_txPIO, _txSM, false);
    }
    if (_rx != NOPIN) {
        pio_sm_set_enabled(_rxPIO, _rxSM, false);
        _pioSP[pio_get_index(_rxPIO)][_rxSM] = nullptr;
        // If no more active, disable the IRQ
        auto pioNum = pio_get_index(_rxPIO);
        bool used = false;
        for (int i = 0; i < 4; i++) {
            used = used || !!_pioSP[pioNum][i];
        }
        if (!used) {
            auto irqno = pioNum == 0 ? PIO0_IRQ_0 : PIO1_IRQ_0;
            irq_set_enabled(irqno, false);
        }
    }
    _running = false;
}

int SerialPIO::peek() {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_rx == NOPIN)) {
        return -1;
    }
    // If there's something in the FIFO now, just peek at it
    if (_writer != _reader) {
        return _queue[_reader];
    }
    return -1;
}

int SerialPIO::read() {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_rx == NOPIN)) {
        return -1;
    }
    if (_writer != _reader) {
        auto ret = _queue[_reader];
        asm volatile("" ::: "memory"); // Ensure the value is read before advancing
        auto next_reader = (_reader + 1) % _fifoSize;
        asm volatile("" ::: "memory"); // Ensure the reader value is only written once, correctly
        _reader = next_reader;
        return ret;
    }
    return -1;
}

bool SerialPIO::overflow() {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_rx == NOPIN)) {
        return false;
    }

    bool hold = _overflow;
    _overflow = false;
    return hold;
}

int SerialPIO::available() {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_rx == NOPIN)) {
        return 0;
    }
    return (_writer - _reader) % _fifoSize;
}

int SerialPIO::availableForWrite() {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_tx == NOPIN)) {
        return 0;
    }
    return 8 - pio_sm_get_tx_fifo_level(_txPIO, _txSM);
}

void SerialPIO::flush() {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_tx == NOPIN)) {
        return;
    }
    while (!pio_sm_is_tx_fifo_empty(_txPIO, _txSM)) {
        delay(1); // Wait for all FIFO to be read
    }
    // Could have 1 byte being transmitted, so wait for bit times
    delay((1000 * (_txBits + 1)) / _baud);
}

size_t SerialPIO::write(uint8_t c) {
    CoreMutex m(&_mutex);
    if (!_running || !m || (_tx == NOPIN)) {
        return 0;
    }

    uint32_t val = c;
    if (_parity == UART_PARITY_NONE) {
        val |= 7 << _bits; // Set 2 stop bits, the HW will only transmit the required number
    } else if (_parity == UART_PARITY_EVEN) {
        val |= ::_parity(_bits, c) << _bits;
        val |= 7 << (_bits + 1);
    } else {
        val |= (1 ^ ::_parity(_bits, c)) << _bits;
        val |= 7 << (_bits + 1);
    }
    val <<= 1;  // Start bit = low

    pio_sm_put_blocking(_txPIO, _txSM, val);

    return 1;
}

SerialPIO::operator bool() {
    return _running;
}
