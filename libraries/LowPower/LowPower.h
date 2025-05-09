/*
 * Copyright (c) 2024 Maximilian Gerhardt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once
#include <Arduino.h>
#include "utility/sleep.h"

class LowPowerClass {
public:
    /**
     * Set the oscillator type that is shutdown during sleep and re-enabled after.
     * The default is "crystal oscillator" (DORMANT_SOURCE_XOSC).
     */
    void setOscillatorType(dormant_source_t oscillator);
    /**
     * Put the chip in "DORMANT" mode and wake up from a GPIO pin.
     * The "wakeup" type can e.g. be "FALLING", so that a falling edge on that GPIO
     * wakes the chip up again.
     * This cannot be "CHANGE"
     */
    void dormantUntilGPIO(pin_size_t wakeup_gpio, PinStatus wakeup_type);
    /**
     * Put the chip in "DORMANT" for a specified amount of time.
     * Note that this does not work on RP2040 chips, unless you connect a 32.768kHz
     * oscillator to specific pins. (TODO exact documentation)
     */
    void dormantFor(uint32_t milliseconds);

private:
    // sane default value, most boards run on the crystal oscillator
    dormant_source_t oscillator = DORMANT_SOURCE_XOSC;
};

extern LowPowerClass LowPower;