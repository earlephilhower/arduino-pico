/*
    LowPower - sleep and dormant modes for the Raspberry Pi Pico (RP2040/RP2350)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#include "LowPower.h"
#include <pico/low_power.h>
#include <hardware/gpio.h>
#include <pico/error.h>
#include <hardware/clocks.h>
#include <hardware/structs/iobank0.h>
#include <hardware/structs/padsbank0.h>
#include <CoreClock.h>
#include <hardware/irq.h>
#include <hardware/sync.h>
#include <hardware/timer.h>
#if defined(PICO_CYW43_SUPPORTED)
#include <pico/cyw43_arch.h>
// The WiFi chip is powered (and its driver running) while the core drives WL_ON high: set when the driver first brings the chip up,
// cleared by cyw43_arch_deinit().  The core initialises the driver at boot on every Pico W sketch, so "initialised" alone says nothing.
static bool cyw43Powered() {
    return cyw43_is_initialized(&cyw43_state) && gpio_get_out_level(CYW43_PIN_WL_REG_ON);
}
#endif
#ifdef __riscv
#include <hardware/riscv.h>
#else
#include <hardware/structs/scb.h>
#endif

// Light sleep clock gating (PLLs, ROSC, USB and ADC clocks stopped, see below): RP2040, and RP2350 on its ARM cores.  The RP2350 RISC-V
// cores keep the SDK's plain sleep (not re-tested with the gating).
#if PICO_RP2040 || !defined(__riscv)
#define LP_LIGHT_SLEEP_CLOCKS 1
#else
#define LP_LIGHT_SLEEP_CLOCKS 0
#endif
#if LP_LIGHT_SLEEP_CLOCKS
#include <hardware/pll.h>
#include <hardware/timer.h>
#include <hardware/uart.h>
#include <hardware/xosc.h>
#include <hardware/structs/rosc.h>
#if defined(NO_USB)
#include <hardware/structs/usb.h>
#include <hardware/structs/resets.h>
#endif
#include <pico/runtime_init.h>
#if !defined(NO_USB) && !defined(USE_TINYUSB)
#include <tusb.h>
#endif
#endif

// Present only if the sketch defines them (same test main.cpp uses)
extern void setup1() __attribute__((weak));
extern void loop1() __attribute__((weak));

LowPowerClass LowPower;

#if LP_LIGHT_SLEEP_CLOCKS
// Light sleep (sleepFor / sleepUntilPin).  The SDK sleep only gates peripheral clocks (SLEEP_ENx) and leaves
// PLL_SYS, PLL_USB, the ring oscillator and clk_usb/clk_adc running: datasheet 2.15.2.5, "The PLLs are not affected by SLEEP mode.  If the
// user wants to save power in SLEEP mode then all clock generators must be switched away from the PLLs and they must be stopped in
// software".  That costs about 6.6 mA on a Pico (8.0 mA board current in sleep, 1.4 mA with the clocks stopped).  So: clk_sys and
// clk_peri from the 12 MHz crystal, ring oscillator off, PLL_SYS stopped; afterwards the clocks are brought back and leave() re-applies the
// core's F_CPU.
// Without a USB stack (NO_USB), or in a USB build while no host is using USB, clk_adc, clk_usb and PLL_USB are stopped too (1.3 mA board current; 0.85 mA in
// NO_USB builds, where the USB pins are pulled down as well, see usbPhyPullDown();
// restore with runtime_init_clocks(), costs about 1.3 ms of wake latency).  In a USB build while a host is using the device, PLL_USB and clk_usb keep
// running so the connection survives the sleep (3.1 mA; nothing re-enumerates).  "In use" = the USB stack sees the bus connected (reset seen, after
// plug-in or tud_connect()) and not suspended; a detached device (tud_disconnect()), an unplugged cable or a suspended host count as not in use.  The
// check is made at every sleep entry and every wake restores all clocks first, so a cable plugged in during a sleep enumerates after the wake.
// Errata RP2040-E16 asks for clk_sys >= 1.1 x clk_usb while USB is in use; in sleep the CPU and the USB interrupts are off and the clocks are
// restored before any USB code runs again.  With Adafruit TinyUSB (USE_TINYUSB) the stack state is not available here: USB is always treated as in use.
static uint32_t savedRoscCtrl;

static void waitUartsIdle() {
    // an in-flight byte would be clocked out at the wrong rate once clk_peri changes
    uart_inst_t *const uarts[] = {uart0, uart1};
    for (uart_inst_t *u : uarts) {
        uint32_t t0 = time_us_32();
        while ((uart_get_hw(u)->cr & UART_UARTCR_UARTEN_BITS) && (uart_get_hw(u)->fr & UART_UARTFR_BUSY_BITS) && time_us_32() - t0 < 50000) {
        }
    }
}

// everything stopped: USB clocks and both PLLs as well
static void clocksDownAll() {
    waitUartsIdle();
    clock_configure_undivided(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0, XOSC_HZ);
    clock_configure_undivided(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, XOSC_HZ);
    clock_stop(clk_adc);
    clock_stop(clk_usb);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, XOSC_HZ, XOSC_HZ);
    pll_deinit(pll_sys);
    pll_deinit(pll_usb);
    savedRoscCtrl = rosc_hw->ctrl;
    rosc_hw->ctrl = (savedRoscCtrl & ~ROSC_CTRL_ENABLE_BITS) | (ROSC_CTRL_ENABLE_VALUE_DISABLE << ROSC_CTRL_ENABLE_LSB);
    while (rosc_hw->status & ROSC_STATUS_STABLE_BITS) {
    }
}

static void clocksUpAll() {
    rosc_hw->ctrl = savedRoscCtrl;
    while (!(rosc_hw->status & ROSC_STATUS_STABLE_BITS)) {
    }
    runtime_init_clocks();
}

#if defined(NO_USB)
// Without a USB stack nothing drives the USB pins D+ / D-, and the USB PHY's receivers sit on floating inputs: about 0.25 mA each on a
// Pico (measured).  For the length of the sleep the PHY's own 15 k pull-downs are switched on (host-like: a connected host sees
// nothing attach); the register values are put back after the wake.  Skipped if the sketch uses the USB controller itself (pull-up enabled)
// or the controller is held in reset.
static uint32_t savedPhyDirect, savedPhyOverride;
static bool phyPulled;

static void usbPhyPullDown() {
    phyPulled = false;
    if ((resets_hw->reset_done & RESETS_RESET_USBCTRL_BITS) && !(usb_hw->sie_ctrl & USB_SIE_CTRL_PULLUP_EN_BITS)) {
        savedPhyDirect = usb_hw->phy_direct;
        savedPhyOverride = usb_hw->phy_direct_override;
        usb_hw->phy_direct = savedPhyDirect | USB_USBPHY_DIRECT_DP_PULLDN_EN_BITS | USB_USBPHY_DIRECT_DM_PULLDN_EN_BITS;
        usb_hw->phy_direct_override = savedPhyOverride | USB_USBPHY_DIRECT_OVERRIDE_DP_PULLDN_EN_OVERRIDE_EN_BITS |
                                      USB_USBPHY_DIRECT_OVERRIDE_DM_PULLDN_EN_OVERRIDE_EN_BITS;
        phyPulled = true;
    }
}

static void usbPhyRestore() {
    if (phyPulled) {
        usb_hw->phy_direct_override = savedPhyOverride;
        usb_hw->phy_direct = savedPhyDirect;
        phyPulled = false;
    }
}

static inline void lightSleepClocksDown() {
    usbPhyPullDown();
    clocksDownAll();
}

static inline void lightSleepClocksUp() {
    clocksUpAll();
    usbPhyRestore();
}
#else
// USB clocks (PLL_USB, clk_usb) kept
static void clocksDownKeepUsb() {
    waitUartsIdle();
    // clk_sys/clk_peri from the crystal; PLL_USB and clk_usb stay
    clock_configure_undivided(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, XOSC_HZ);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, XOSC_HZ, XOSC_HZ);
    clock_stop(clk_adc);
    pll_deinit(pll_sys);
    savedRoscCtrl = rosc_hw->ctrl;
    rosc_hw->ctrl = (savedRoscCtrl & ~ROSC_CTRL_ENABLE_BITS) | (ROSC_CTRL_ENABLE_VALUE_DISABLE << ROSC_CTRL_ENABLE_LSB);
    while (rosc_hw->status & ROSC_STATUS_STABLE_BITS) {
    }
}

static void clocksUpKeepUsb() {
    rosc_hw->ctrl = savedRoscCtrl;
    while (!(rosc_hw->status & ROSC_STATUS_STABLE_BITS)) {
    }
    // only PLL_SYS and what hangs on it come back; PLL_USB and clk_usb were never touched
    pll_init(pll_sys, PLL_SYS_REFDIV, PLL_SYS_VCO_FREQ_HZ, PLL_SYS_POSTDIV1, PLL_SYS_POSTDIV2);
    clock_configure_undivided(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0, XOSC_HZ);
    clock_configure_undivided(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX, CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, SYS_CLK_HZ);
    clock_configure_undivided(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, SYS_CLK_HZ);
    clock_configure_undivided(clk_adc, 0, CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, USB_CLK_HZ);
}

static bool usbGated; // this sleep stopped the USB clocks too

static bool usbInUse() {
#if defined(USE_TINYUSB)
    return true;
#else
    return tud_connected() && !tud_suspended();
#endif
}

static void lightSleepClocksDown() {
    usbGated = !usbInUse();
    if (usbGated) {
        clocksDownAll();
    } else {
        clocksDownKeepUsb();
    }
}

static void lightSleepClocksUp() {
    if (usbGated) {
        clocksUpAll();
    } else {
        clocksUpKeepUsb();
    }
}
#endif
#else
static inline void lightSleepClocksDown() {}
static inline void lightSleepClocksUp() {}
#endif

// Saved pin configuration (whole IO_BANK0 CTRL and PADS_BANK0 registers) for pins that get parked
static uint32_t savedIoCtrl[NUM_BANK0_GPIOS];
static uint32_t savedPad[NUM_BANK0_GPIOS];

// Map Arduino PinStatus to the SDK's (edge, high) pair
static bool mapPinStatus(PinStatus mode, bool *edge, bool *high) {
    switch (mode) {
    case LOW:
        *edge = false;
        *high = false;
        return true;
    case HIGH:
        *edge = false;
        *high = true;
        return true;
    case RISING:
        *edge = true;
        *high = true;
        return true;
    case FALLING:
        *edge = true;
        *high = false;
        return true;
    default:
        return false; // CHANGE is not supported by the SDK
    }
}

void LowPowerClass::enter() {
    if (setup1 || loop1) {
        rp2040.idleOtherCore();
    }
}

void LowPowerClass::leave() {
    // The SDK wakes into its default clocks (125 MHz RP2040 / 150 MHz RP2350).  Dormant
    // always ends there, and so does RP2040 light sleep (the clocks are lowered, see lightSleepClocksDown;
    // the USB variant brings PLL_SYS back at the SDK default); RP2350 light sleep only gates clocks.
    // Re-apply the core's F_CPU/VREG/PSRAM setup only when clk_sys is not where boot left it.
    if (clock_get_hz(clk_sys) != (uint32_t)F_CPU) {
        __applyCoreClockConfig();
    }
    // Pins go back after the clocks, before the sketch (and core 1) runs again
    unparkPins();
    if (setup1 || loop1) {
        rp2040.resumeOtherCore();
    }
}

void LowPowerClass::keepPins(uint64_t mask) {
    _keepMask = mask;
}

void LowPowerClass::setPinRestore(bool restore) {
    _restore = restore;
}

uint32_t LowPowerClass::lastSleepMs() const {
    return _lastSleepMs;
}

// Save every pin that is about to be parked, then park them.  The SDK parking
// (low_power_set_pins_low_leakage_exclude_mask64) changes, per pin: PADS PUE+PDE (pulls off),
// PADS IE (input off) and IO CTRL OEOVER (output disabled).  We save the whole PADS and CTRL
// registers, which covers those fields plus function select, drive strength, slew, schmitt,
// output-disable and (RP2350) the pad isolation bit.  SIO direction/output level are not
// touched by parking so they need no saving.
void LowPowerClass::parkPins(uint64_t keep) {
    if (_restore) {
        for (uint p = 0; p < NUM_BANK0_GPIOS; p++) {
            if (!((keep >> p) & 1)) {
                savedPad[p] = pads_bank0_hw->io[p];
                savedIoCtrl[p] = io_bank0_hw->io[p].ctrl;
            }
        }
        _saved = true;
        _savedKeep = keep;
    }
    low_power_set_pins_low_leakage_exclude_mask64(keep);
}

void LowPowerClass::unparkPins() {
    if (!_saved) {
        return;
    }
    for (uint p = 0; p < NUM_BANK0_GPIOS; p++) {
        if (!((_savedKeep >> p) & 1)) {
            // pad first (pulls, input enable), then the IO mux/overrides (output enable)
            pads_bank0_hw->io[p] = savedPad[p];
            io_bank0_hw->io[p].ctrl = savedIoCtrl[p];
        }
    }
    _saved = false;
}

uint64_t LowPowerClass::parkMask(int wakePin) {
    uint64_t keep = _keepMask;
    if (wakePin >= 0) {
        keep |= 1ULL << wakePin;
    }
    // Leave I2C pins alone: with input/pulls off the controller sees a bus glitch and, once the pads
    // come back, every transfer times out until Wire.end()/begin().  The bus idles high, so
    // keeping the pads costs no current.  Also leave Serial1/Serial2 pins alone so TX doesn't float.
    for (uint p = 0; p < NUM_BANK0_GPIOS; p++) {
        if (gpio_get_function(p) == GPIO_FUNC_UART || gpio_get_function(p) == GPIO_FUNC_I2C) {
            keep |= 1ULL << p;
        }
    }
#if defined(PICO_CYW43_SUPPORTED)
    // Pico W / Pico 2 W: with the WiFi chip powered, its pins stay as they are (WL_ON, the PIO SPI data / IRQ, CS and clock).  Parked, WL_ON
    // floats and the CYW43 loses its firmware and the driver state with it (measured: no ping, no reconnect until a reset).
    if (cyw43Powered()) {
        keep |= (1ULL << CYW43_PIN_WL_REG_ON) | (1ULL << CYW43_PIN_WL_DATA_OUT) | (1ULL << CYW43_PIN_WL_DATA_IN) | (1ULL << CYW43_PIN_WL_HOST_WAKE) |
                (1ULL << CYW43_PIN_WL_CS) | (1ULL << CYW43_PIN_WL_CLOCK);
    }
#endif
    return keep;
}

// ---- light sleep until a pin, with or without a timeout.  The SDK has no sleep with two wake sources, so this is the SDK's sleep_until_timer
// and sleep_until_gpio_pin_state in one: a timer alarm (only with a timeout) and a GPIO IRQ both set a flag, the WFI loop ends on either.  The
// system timer clock is kept in both forms, so millis() stays continuous and the duration is measured (the SDK's pin sleep gates it).  The GPIO
// IRQ uses a raw IO_IRQ_BANK0 handler, so the SDK's GPIO callback
// (which attachInterrupt() relies on) is not replaced.  The SDK's own sleep_until_gpio_pin_state() ends by setting that callback to NULL, which
// removes the shared handler: the next edge on an attachInterrupt() pin then raises IO_IRQ_BANK0 with no handler and the sketch hangs in the
// SDK's unhandled-IRQ spin.  That is why sleepUntilPin(pin, mode) does not call it.
static volatile bool lpPinFired, lpTimerFired;
static uint lpPin;
static uint32_t lpPinEvent;

static void lpGpioHandler() {
    if (gpio_get_irq_event_mask(lpPin) & lpPinEvent) {
        gpio_set_irq_enabled(lpPin, lpPinEvent, false); // acknowledges, and stops a level IRQ from firing again
        lpPinFired = true;
    }
}

static void lpAlarmHandler(uint) {
    lpTimerFired = true;
}

static void lpKeepClocks(clock_dest_bitset_t *keep) {
#if PICO_RP2040
    clock_dest_bitset_add(keep, CLK_DEST_SYS_TIMER);
#else
    clock_dest_bitset_add(keep, timer_get_index(timer_hw) ? CLK_DEST_SYS_TIMER1 : CLK_DEST_SYS_TIMER0);
    clock_dest_bitset_add(keep, CLK_DEST_REF_TICKS);
#endif
    // as the SDK's sleep keeps them: UART0 (Serial1 keeps receiving, so a byte can be the wake source) and the USB controller (connection survives)
    clock_dest_bitset_add(keep, CLK_DEST_SYS_UART0);
    clock_dest_bitset_add(keep, CLK_DEST_PERI_UART0);
#if !defined(NO_USB)
    clock_dest_bitset_add(keep, CLK_DEST_SYS_USBCTRL);
#if PICO_RP2040
    clock_dest_bitset_add(keep, CLK_DEST_USB_USBCTRL);
#else
    clock_dest_bitset_add(keep, CLK_DEST_USB);
#endif
#endif
}

// every interrupt except `keep0` and `keep1` is disabled while asleep, so nothing but the two wake sources runs
static uint32_t lpIrqSaved[(NUM_IRQS + 31) / 32];

static void lpDisableOtherIrqs(uint keep0, uint keep1) {
    for (uint n = 0; n < (NUM_IRQS + 31) / 32; n++) {
        uint32_t m = irq_get_mask_n(n);
        if (keep0 / 32 == n) {
            m &= ~(1u << (keep0 % 32));
        }
        if (keep1 / 32 == n) {
            m &= ~(1u << (keep1 % 32));
        }
        lpIrqSaved[n] = m;
        irq_set_mask_n_enabled(n, m, false);
    }
}

static void lpRestoreIrqs() {
    for (uint n = 0; n < (NUM_IRQS + 31) / 32; n++) {
        irq_set_mask_n_enabled(n, lpIrqSaved[n], true);
    }
}

static void lpDeepSleepOn() {
#ifdef __riscv
    uint32_t bits = RVCSR_MSLEEP_POWERDOWN_BITS;
    if (!get_core_num()) {
        bits |= RVCSR_MSLEEP_DEEPSLEEP_BITS; // see errata RP2350-E4
    }
    riscv_set_csr(RVCSR_MSLEEP_OFFSET, bits);
#else
    scb_hw->scr |= ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);
#endif
}

static void lpDeepSleepOff() {
#ifdef __riscv
    riscv_clear_csr(RVCSR_MSLEEP_OFFSET, RVCSR_MSLEEP_POWERDOWN_BITS | RVCSR_MSLEEP_DEEPSLEEP_BITS);
#else
    scb_hw->scr &= ~ARM_CPU_PREFIXED(SCR_SLEEPDEEP_BITS);
#endif
}

int LowPowerClass::sleepUntilPin(pin_size_t pin, PinStatus mode, uint32_t timeoutMs) {
    if (!timeoutMs) {
        _lastSleepMs = 0;
        _wakeReason = WAKE_NONE;
        return PICO_ERROR_INVALID_ARG;
    }
    return sleepUntilPinImpl(pin, mode, timeoutMs, true);
}

int LowPowerClass::sleepUntilPin(pin_size_t pin, PinStatus mode) {
    return sleepUntilPinImpl(pin, mode, 0, false);
}

int LowPowerClass::sleepUntilPinImpl(pin_size_t pin, PinStatus mode, uint32_t timeoutMs, bool timed) {
    bool edge, high;
    _lastSleepMs = 0;
    _wakeReason = WAKE_NONE;
#ifdef __FREERTOS
    return PICO_ERROR_NOT_PERMITTED; // FreeRTOS owns the tick, the second core and the WiFi driver's tasks: not supported, see docs/lowpower.rst
#endif
    if (pin >= NUM_BANK0_GPIOS || !mapPinStatus(mode, &edge, &high)) {
        return PICO_ERROR_INVALID_ARG;
    }
    int alarm = -1;
    if (timed) {
        alarm = timer_hardware_alarm_claim_unused(timer_hw, false);
        if (alarm < 0) {
            return PICO_ERROR_INSUFFICIENT_RESOURCES;
        }
    }
    const uint32_t event = edge ? (high ? GPIO_IRQ_EDGE_RISE : GPIO_IRQ_EDGE_FALL) : (high ? GPIO_IRQ_LEVEL_HIGH : GPIO_IRQ_LEVEL_LOW);
    clock_dest_bitset_t keep = clock_dest_bitset_none();
    lpKeepClocks(&keep);

    enter();
#if defined(PICO_CYW43_SUPPORTED)
    // As in sleepFor(): a CYW43 interrupt (WiFi or, much more often, Bluetooth traffic) shares IO_IRQ_BANK0 with the wake pin and would run the driver's worker
    // while the clocks are lowered, whose PIO SPI transfer then fails or hangs (Bluetooth: advertising and the stack are lost).  Holding the async context's lock
    // keeps the worker off until the clocks are back.
    async_context_t *cyw43Ctx = cyw43Powered() ? cyw43_arch_async_context() : nullptr;
    if (cyw43Ctx) {
        async_context_acquire_lock_blocking(cyw43Ctx);
    }
#endif
    lpPinFired = false;
    lpTimerFired = false;
    lpPin = pin;
    lpPinEvent = event;
    const uint64_t t0 = time_us_64();
    if (timed) {
        timer_hardware_alarm_set_callback(timer_hw, alarm, lpAlarmHandler);
    }
    gpio_set_input_enabled(pin, true);
    gpio_add_raw_irq_handler(pin, lpGpioHandler);
    gpio_set_irq_enabled(pin, event, true);
    irq_set_enabled(IO_IRQ_BANK0, true);
    if (timed && timer_hardware_alarm_set_target(timer_hw, alarm, make_timeout_time_ms(timeoutMs))) {
        lpTimerFired = true; // the time has passed already
    }
    lpDisableOtherIrqs(IO_IRQ_BANK0, timed ? timer_hardware_alarm_get_irq_num(timer_hw, alarm) : (uint)IO_IRQ_BANK0);
    lightSleepClocksDown();
    clock_gate_sleep_en(&keep);
    lpDeepSleepOn();
    // Same loop as the SDK's sleeps: WFI with interrupts enabled, so the wake IRQ is taken at once.  (Masking interrupts around the check,
    // to close the check-then-WFI window, made the CPU run on right after the wake while the gated clocks were still restarting: intermittent
    // hard fault right after the WFI, seen with a USB host enumerating.  The window is a few cycles; the timeout form would only wake late.)
    while (!lpPinFired && !lpTimerFired) {
        __wfi();
    }
    lpDeepSleepOff();
    clock_dest_bitset_t all = clock_dest_bitset_all();
    clock_gate_sleep_en(&all);
    lightSleepClocksUp();
    lpRestoreIrqs();
    const uint64_t t1 = time_us_64();
    // disarm both sources; a late edge or alarm that raced the other one is dropped here
    gpio_set_irq_enabled(pin, event, false);
    gpio_remove_raw_irq_handler(pin, lpGpioHandler);
    if (timed) {
        timer_hardware_alarm_cancel(timer_hw, alarm);
        timer_hardware_alarm_set_callback(timer_hw, alarm, nullptr);
        timer_hardware_alarm_unclaim(timer_hw, alarm);
    }
    leave();
#if defined(PICO_CYW43_SUPPORTED)
    if (cyw43Ctx) {
        async_context_release_lock(cyw43Ctx);
    }
#endif
    _wakeReason = lpPinFired ? WAKE_PIN : WAKE_TIMER;
    _lastSleepMs = (uint32_t)((t1 - t0 + 500) / 1000);
    return 0;
}

int LowPowerClass::sleepFor(uint32_t ms) {
    _lastSleepMs = 0;
    _wakeReason = WAKE_NONE;
#ifdef __FREERTOS
    return PICO_ERROR_NOT_PERMITTED; // FreeRTOS owns the tick, the second core and the WiFi driver's tasks: not supported, see docs/lowpower.rst
#endif
    if (!ms) {
        return 0;
    }
    enter();
#if defined(PICO_CYW43_SUPPORTED)
    // The SDK enables the other interrupts again before it returns, i.e. while the clocks are still lowered.  A pending CYW43 interrupt then
    // runs the WiFi driver's worker, whose PIO SPI transfer never completes at that clock: hang.  Holding the async context's lock keeps the
    // worker off until the clocks are back.
    async_context_t *cyw43Ctx = cyw43Powered() ? cyw43_arch_async_context() : nullptr;
    if (cyw43Ctx) {
        async_context_acquire_lock_blocking(cyw43Ctx);
    }
#endif
    lightSleepClocksDown();
    int rc = low_power_sleep_for_ms(ms, nullptr, true);
    lightSleepClocksUp();
    leave();
#if defined(PICO_CYW43_SUPPORTED)
    if (cyw43Ctx) {
        async_context_release_lock(cyw43Ctx);
    }
#endif
    _lastSleepMs = rc ? 0 : ms;
    _wakeReason = rc ? WAKE_NONE : WAKE_TIMER;
    return rc;
}

int LowPowerClass::dormantUntilPin(pin_size_t pin, PinStatus mode) {
    bool edge, high;
    _lastSleepMs = 0;
    _wakeReason = WAKE_NONE;
#ifdef __FREERTOS
    return PICO_ERROR_NOT_PERMITTED; // FreeRTOS owns the tick, the second core and the WiFi driver's tasks: not supported, see docs/lowpower.rst
#endif
    if (pin >= NUM_BANK0_GPIOS || !mapPinStatus(mode, &edge, &high)) {
        return PICO_ERROR_INVALID_ARG;
    }
    enter();
    parkPins(parkMask(pin));
    // GPIO wake needs no running timer clock, so the plain crystal oscillator source is used on both chips
    int rc = low_power_dormant_until_gpio_pin_state(pin, edge, high, DORMANT_CLOCK_SOURCE_XOSC, nullptr);
    leave();
    _lastSleepMs = rc ? 0 : SLEEP_UNKNOWN;
    _wakeReason = rc ? WAKE_NONE : WAKE_PIN;
    return rc;
}

int LowPowerClass::dormantFor(uint32_t ms) {
#if PICO_RP2040
    // ROSC source = RTC fed from an external clock; the SDK returns
    // PICO_ERROR_PRECONDITION_NOT_MET unless low_power_set_external_clock_source() was called.
    const dormant_clock_source_t src = DORMANT_CLOCK_SOURCE_ROSC;
#else
    const dormant_clock_source_t src = DORMANT_CLOCK_SOURCE_LPOSC;
#endif
    _lastSleepMs = 0;
    _wakeReason = WAKE_NONE;
#ifdef __FREERTOS
    return PICO_ERROR_NOT_PERMITTED; // FreeRTOS owns the tick, the second core and the WiFi driver's tasks: not supported, see docs/lowpower.rst
#endif
    enter();
    parkPins(parkMask(-1));
    int rc = low_power_dormant_for_ms(ms, src, nullptr);
    leave();
    _lastSleepMs = rc ? 0 : ms;
    _wakeReason = rc ? WAKE_NONE : WAKE_TIMER;
    return rc;
}
