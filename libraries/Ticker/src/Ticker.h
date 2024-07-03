/*
    Ticker.h - esp32 library that calls functions periodically
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

#pragma once

#include <Arduino.h>
#include <functional>

// The templated function casting <4 bytes to a 4-byte variable causes
// warnings on GCC 12 (but no actual problem since the smallest arg
// that can be physically passed is 32-bits).  Disable that specific
// warning for this file only.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

class Ticker {
public:
    Ticker();
    ~Ticker();

    typedef void (*callback_with_arg_t)(void *);
    typedef std::function<void(void)> callback_function_t;

    bool attach(float seconds, callback_function_t callback) {
        _callback_function = std::move(callback);
        return _attach_us(1000000ULL * seconds, true, _static_callback, this);
    }

    bool attach_ms(uint64_t milliseconds, callback_function_t callback) {
        _callback_function = std::move(callback);
        return _attach_us(1000ULL * milliseconds, true, _static_callback, this);
    }

    bool attach_us(uint64_t micros, callback_function_t callback) {
        _callback_function = std::move(callback);
        return _attach_us(micros, true, _static_callback, this);
    }

    template<typename TArg> bool attach(float seconds, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        // C-cast serves two purposes:
        // static_cast for smaller integer types,
        // reinterpret_cast + const_cast for pointer types
        return _attach_us(1000000ULL * seconds, true, reinterpret_cast<callback_with_arg_t>(callback), reinterpret_cast<void *>(arg));
    }

    template<typename TArg> bool attach_ms(uint64_t milliseconds, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        return _attach_us(1000ULL * milliseconds, true, reinterpret_cast<callback_with_arg_t>(callback), reinterpret_cast<void *>(arg));
    }

    template<typename TArg> bool attach_us(uint64_t micros, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        return _attach_us(micros, true, reinterpret_cast<callback_with_arg_t>(callback), reinterpret_cast<void *>(arg));
    }

    bool once(float seconds, callback_function_t callback) {
        _callback_function = std::move(callback);
        return _attach_us(1000000ULL * seconds, false, _static_callback, this);
    }

    bool once_ms(uint64_t milliseconds, callback_function_t callback) {
        _callback_function = std::move(callback);
        return _attach_us(1000ULL * milliseconds, false, _static_callback, this);
    }

    bool once_us(uint64_t micros, callback_function_t callback) {
        _callback_function = std::move(callback);
        return _attach_us(micros, false, _static_callback, this);
    }

    template<typename TArg> bool once(float seconds, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        return _attach_us(1000000ULL * seconds, false, reinterpret_cast<callback_with_arg_t>(callback), reinterpret_cast<void *>(arg));
    }

    template<typename TArg> bool once_ms(uint64_t milliseconds, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        return _attach_us(1000ULL * milliseconds, false, reinterpret_cast<callback_with_arg_t>(callback), reinterpret_cast<void *>(arg));
    }

    template<typename TArg> bool once_us(uint64_t micros, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(void *), "attach() callback argument size must be <= sizeof(void*)");
        return _attach_us(micros, false, reinterpret_cast<callback_with_arg_t>(callback), reinterpret_cast<void *>(arg));
    }

    void detach();
    bool active() const;

protected:
    static void _static_callback(void *arg);
    static bool _repeating_callback(repeating_timer_t *arg);
    static int64_t _alarm_callback(alarm_id_t id, void *user_data);

    // No arguments
    callback_function_t _callback_function = nullptr;
    // Arguments
    callback_with_arg_t _callback_with_arg = nullptr;
    void * _callback_arg;

    alarm_id_t _alarm_id;
    repeating_timer_t *_repeating_timer;

private:
    bool _attach_us(uint64_t micros, bool repeat, callback_with_arg_t callback, void *arg);
};
#pragma GCC diagnostic pop
