/*
    Lockless single reader, single writer queue

    Implements a queue of elements that can be safely accessed from two
    different contexts in parallel without any locking ever as long as
    there is only one ctx which always writes, and another which always
    reads (like a serial port SW FIFO coming off of a HW FIFO).

    Copyright (c) 2026 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#pragma once

template <typename T>
class LocklessQueue {
public:
    LocklessQueue(size_t size) {
        _queue = new T[size];
        _writer = 0;
        _reader = 0;
        _fifoSize = size;
    }

    ~LocklessQueue() {
        delete[] _queue;
    }

    // How many elements we can read
    int available() {
        // Do MOD without division
        size_t s = _fifoSize + _writer - _reader;
        while (s >= _fifoSize) {
            s -= _fifoSize;
        }
        return s;
    }

    // Store an element
    bool write(T val) {
        auto next_writer = _writer + 1;
        if (next_writer == _fifoSize) {
            next_writer = 0;
        }
        if (next_writer != _reader) {
            _queue[_writer] = val;
            asm volatile("" ::: "memory"); // Ensure the queue is written before the written count advances
            _writer = next_writer;
            return true;
        }
        return false;
    }

    // Examine next element to read
    bool peek(T *val) {
        if (_writer == _reader) {
            return false;
        }
        *val = _queue[_reader];
        return true;
    }

    // Are there no elements left?
    bool empty() {
        return _writer == _reader;
    }

    // Read a single element if present
    bool read(T *dst) {
        if (_writer == _reader) {
            return false;
        }
        auto ret = _queue[_reader];
        asm volatile("" ::: "memory"); // Ensure the value is read before advancing
        auto next_reader = _reader + 1;
        if (next_reader == _fifoSize) {
            next_reader = 0;
        }
        asm volatile("" ::: "memory"); // Ensure the reader value is only written once, correctly
        _reader = next_reader;
        *dst = ret;
        return true;
    }

    void reset() {
        _writer = 0;
        _reader = 0;
    }

private:
    uint32_t _writer;   // Must be atomic writable, so 32b
    uint32_t _reader;   // Must be atomic writable, so 32b
    size_t   _fifoSize; // # of elements
    T       *_queue;    // Actual data
};
