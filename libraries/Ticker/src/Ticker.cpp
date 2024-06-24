/*
    Ticker.cpp - RP2040 library that calls functions periodically
    Ported 2024 to the RP2040 by Earle F. Philhower, III <earlephilhower@yahoo.com>

    Copyright (c) 2017 Bert Melis. All rights reserved.

    Based on the original work of:
    Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
    The original version is part of the esp8266 core for Arduino environment.

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

#include "Ticker.h"

Ticker::Ticker() : _callback_function(nullptr), _callback_with_arg(nullptr), _alarm_id(-1), _repeating_timer(nullptr) { }

Ticker::~Ticker() {
    detach();
}

bool Ticker::_attach_us(uint64_t micros, bool repeat, callback_with_arg_t callback, void *arg) {
    detach();
    _callback_with_arg = callback;
    _callback_arg = arg;
    if (repeat) {
        _repeating_timer = new repeating_timer_t;
        _repeating_timer->user_data = (void *)this;
        return add_repeating_timer_us(micros, _repeating_callback, this, _repeating_timer);
    } else {
        _alarm_id = add_alarm_in_us(micros, _alarm_callback, this, true);
        return _alarm_id != -1;
    }
}

void Ticker::detach() {
    if (_repeating_timer) {
        cancel_repeating_timer(_repeating_timer);
        delete _repeating_timer;
    } else if (_alarm_id != -1) {
        cancel_alarm(_alarm_id);
    }
    _repeating_timer = nullptr;
    _alarm_id = -1;
    _callback_with_arg = nullptr;
}

bool Ticker::active() const {
    if (!_repeating_timer && (_alarm_id == -1)) {
        return false;
    }
    return true;
}

void Ticker::_static_callback(void *arg) {
    Ticker *_this = reinterpret_cast<Ticker *>(arg);
    if (_this && _this->_callback_function) {
        _this->_callback_function();
    }
}

bool Ticker::_repeating_callback(repeating_timer_t *arg) {
    Ticker *_this = reinterpret_cast<Ticker *>(arg->user_data);
    if (_this && _this->_callback_function) {
        _this->_callback_with_arg(_this->_callback_arg);
    }
    return true;
}

int64_t Ticker::_alarm_callback(alarm_id_t id, void *user_data) {
    (void) id;
    Ticker *_this = reinterpret_cast<Ticker *>(user_data);
    if (_this && _this->_callback_function) {
        _this->_callback_with_arg(_this->_callback_arg);
    }
    _this->_alarm_id = -1;
    _this->_callback_with_arg = nullptr;
    return 0; // These are only a 1-shot
}
