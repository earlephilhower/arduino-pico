/*
    LowPower - sleep and dormant modes for the Raspberry Pi Pico (RP2040/RP2350)

    Thin Arduino-style wrapper over the Pico SDK `pico_low_power` library.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include <Arduino.h>

/**
    @brief Low-power modes.  All functions return 0 on success or a (negative)
    Pico SDK error code (PICO_ERROR_*, see pico/error.h) on failure.
*/
class LowPowerClass {
public:
    /**
        @brief What ended the last sleep, see wakeReason().  Use it as LowPower.WAKE_PIN (or LowPowerClass::WAKE_PIN),
        the names are not global so they cannot clash with a sketch's own constants.
    */
    enum WakeReason {
        WAKE_NONE,  /**< no sleep yet, or the last call failed or slept nothing */
        WAKE_PIN,   /**< the wake pin condition */
        WAKE_TIMER  /**< the timeout (or the requested time) elapsed */
    };

    /** Value returned by lastSleepMs() when the sleep duration cannot be determined. */
    static constexpr uint32_t SLEEP_UNKNOWN = 0xFFFFFFFFu;

    /**
        @brief Light sleep: clocks gated, chip wakes after a timeout.  RP2040 and RP2350.

        @param [in] ms Time to sleep in milliseconds.
        @return 0 on success, SDK error code otherwise
    */
    int sleepFor(uint32_t ms);

    /**
        @brief Light sleep until a GPIO condition.  RP2040 and RP2350.

        @param [in] pin GPIO to watch
        @param [in] mode LOW, HIGH, RISING or FALLING
        @return 0 on success, SDK error code otherwise
    */
    int sleepUntilPin(pin_size_t pin, PinStatus mode);

    /**
        @brief Light sleep until a GPIO condition or a timeout, whichever comes first.  RP2040 and RP2350.

        Use wakeReason() to find out which one it was.  If the pin is already in the wake
        state when called (LOW or HIGH level), it returns at once with WAKE_PIN.  If the pin
        condition and the timeout happen together either reason may be reported, never both.
        Unlike sleepUntilPin(pin, mode) the system timer keeps running, so millis() includes the
        sleep and lastSleepMs() is the measured duration (rounded to ms).

        @param [in] pin GPIO to watch
        @param [in] mode LOW, HIGH, RISING or FALLING
        @param [in] timeoutMs Longest time to sleep in milliseconds, must not be 0
        @return 0 on success, PICO_ERROR_INVALID_ARG for a bad pin, mode or a timeout of 0,
                other SDK error code otherwise
    */
    int sleepUntilPin(pin_size_t pin, PinStatus mode, uint32_t timeoutMs);

    /**
        @brief Dormant: oscillators stopped, wake only from a GPIO.  RP2040 and RP2350.

        All GPIOs not excluded by keepPins(), not the wake pin and not currently muxed
        to a UART or I2C are parked in a low-leakage state (input and pulls disabled, output
        disconnected) while dormant.  Their previous state is saved first and put back
        after wake, unless setPinRestore(false) was called, in which case they stay
        parked and must be reconfigured with pinMode().

        @param [in] pin GPIO to wake on (set up its pull-up/down with pinMode() beforehand)
        @param [in] mode LOW, HIGH, RISING or FALLING
        @return 0 on success, SDK error code otherwise
    */
    int dormantUntilPin(pin_size_t pin, PinStatus mode);

    /**
        @brief Dormant with a timed wake.

        RP2350: uses the AON timer clocked from the low-power oscillator.
        RP2040: requires an external 32 kHz-class clock registered with the SDK's
        low_power_set_external_clock_source(); without it returns
        PICO_ERROR_PRECONDITION_NOT_MET.  Minimum duration is
        PICO_LOW_POWER_MIN_DORMANT_TIME_MS (2000 ms RP2040, 10 ms RP2350) or
        PICO_ERROR_INVALID_ARG is returned.  Same pin-parking rules as dormantUntilPin().

        @param [in] ms Time to stay dormant in milliseconds
        @return 0 on success, SDK error code otherwise
    */
    int dormantFor(uint32_t ms);

    /**
        @brief GPIOs to leave untouched by the dormant low-leakage parking.

        The wake pin and any pin muxed to a UART or I2C are always left alone.

        @param [in] mask Bit n set = leave GPIO n alone
    */
    void keepPins(uint64_t mask);

    /**
        @brief Choose whether pins parked for dormant are restored on wake.

        Default true: every parked pin gets its pad (pulls, input enable, drive, slew,
        schmitt, ...) and IO (function, overrides) configuration back after wake.
        With false the pins stay in the low-leakage state.

        @param [in] restore true = restore on wake (default), false = leave parked
    */
    void setPinRestore(bool restore);

    /**
        @brief How long the last sleepFor/sleepUntilPin/dormantFor/dormantUntilPin call slept.

        Returns, for the most recent call:
          - sleepFor(ms), dormantFor(ms) that returned 0: the requested duration `ms`
            (not a measurement)
          - either form of sleepUntilPin() that returned 0: the measured duration (the system
            timer keeps running in light sleep)
          - dormantUntilPin() that returned 0: SLEEP_UNKNOWN (0xFFFFFFFF), because there is no
            reliable time reference while dormant (the system timer that millis() uses is stopped)
          - any call that returned an error (or sleepFor(0)): 0, nothing was slept
          - before the first call: 0

        millis() and micros() do NOT include time spent dormant (the timer stops there); light
        sleep (sleepFor, sleepUntilPin) keeps it running.  Add lastSleepMs() yourself after a
        dormant call if you need wall-clock time, where it is known (dormantFor).

        @return milliseconds slept, 0 if none, or SLEEP_UNKNOWN
    */
    uint32_t lastSleepMs() const;

    /**
        @brief What ended the most recent sleep or dormant call.

        sleepFor() and dormantFor() report WAKE_TIMER, sleepUntilPin(pin, mode) and
        dormantUntilPin() WAKE_PIN, sleepUntilPin(pin, mode, timeoutMs) either.  A call that
        returned an error (or sleepFor(0)) leaves WAKE_NONE; so does the time before the first call.

        @return WAKE_NONE, WAKE_PIN or WAKE_TIMER
    */
    WakeReason wakeReason() const {
        return _wakeReason;
    }

private:
    uint64_t _keepMask = 0;
    bool _restore = true;
    bool _saved = false;
    uint64_t _savedKeep = 0;
    uint32_t _lastSleepMs = 0;
    WakeReason _wakeReason = WAKE_NONE;
    int sleepUntilPinImpl(pin_size_t pin, PinStatus mode, uint32_t timeoutMs, bool timed);
    uint64_t parkMask(int wakePin);
    void parkPins(uint64_t keep);
    void unparkPins();
    void enter();
    void leave();
};

extern LowPowerClass LowPower;
