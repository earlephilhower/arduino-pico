/*
 * Copyright (c) 2024 Maximilian Gerhardt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "LowPower.h"
#include <hardware/structs/vreg_and_chip_reset.h>
#include <hardware/structs/syscfg.h>

LowPowerClass LowPower;

void LowPowerClass::setOscillatorType(dormant_source_t oscillator) {
    this->oscillator = oscillator;    
}

static uint32_t pinStatusToGPIOEvent(PinStatus status) {
    switch (status) {
        case HIGH: return IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_HIGH_BITS;
        case LOW: return IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_LOW_BITS;
        case FALLING: return IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS;
        case RISING: return IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS;
        // Change activates on both a rising or a falling edge
        case CHANGE:
            return IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS | 
              IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS;
        default: return 0; // unreachable
    }
}

void LowPowerClass::dormantUntilGPIO(pin_size_t wakeup_gpio, PinStatus wakeup_type){
    sleep_run_from_dormant_source(this->oscillator);
    uint32_t event = pinStatusToGPIOEvent(wakeup_type);

    // turn of brown out detector (BOD), just pisses away power
    vreg_and_chip_reset_hw->bod &= ~VREG_AND_CHIP_RESET_BOD_EN_BITS;
    // set digital voltage to only 0.8V instead of 1.10V. burns less static power
    vreg_and_chip_reset_hw->vreg &= ~VREG_AND_CHIP_RESET_VREG_VSEL_BITS;
    // power down additional stuff
    syscfg_hw->mempowerdown |= 
        SYSCFG_MEMPOWERDOWN_USB_BITS // USB Memory
    ; 

    sleep_goto_dormant_until_pin((uint) wakeup_gpio, event);

    syscfg_hw->mempowerdown &= 
        ~(SYSCFG_MEMPOWERDOWN_USB_BITS) // USB Memory
    ; 
    // We only reach the next line after waking up
    vreg_and_chip_reset_hw->bod |= VREG_AND_CHIP_RESET_BOD_EN_BITS; // turn BOD back on
    // default 1.10V again
    vreg_and_chip_reset_hw->vreg |= VREG_AND_CHIP_RESET_VREG_VSEL_RESET << VREG_AND_CHIP_RESET_VREG_VSEL_LSB;

    sleep_power_up();

    // startup crystal oscillator (?)
#if (defined(PICO_RP2040) && (F_CPU != 125000000)) || (defined(PICO_RP2350) && (F_CPU != 150000000))
    set_sys_clock_khz(F_CPU / 1000, true);
#endif
}

static void sleep_callback(void) { }

// For RP2040 this example needs an external clock fed into the GP20
// Note: Only GP20 and GP22 can be used for clock input, See the GPIO function table in the datasheet.
// You can use another Pico to generate this. See the clocks/hello_gpout example for more details.
// rp2040: clock_gpio_init(21, CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_VALUE_CLK_RTC, 1); // 46875Hz can only export a clock on gpios 21,23,24,25 and only 21 is exposed by Pico
// RP2350 has an LPOSC it can use, so doesn't need this
// also need an initial value like
//   struct timespec ts = { .tv_sec = 1723124088, .tv_nsec = 0 };
//   aon_timer_start(&ts);
void LowPowerClass::dormantFor(uint32_t milliseconds) {
    sleep_run_from_dormant_source(this->oscillator);
    struct timespec ts = {};
    aon_timer_get_time(&ts);
    if (milliseconds >= 1000) {
        ts.tv_sec += milliseconds / 1000;
        milliseconds = milliseconds % 1000;
    }
    ts.tv_nsec += milliseconds * 1000ul * 1000ul;
    sleep_goto_dormant_until(&ts, &sleep_callback);
}
