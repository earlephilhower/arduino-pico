// Rewrite the CYW43 driver to use a native FreeRTOS task and primitives
// It seems hard(impossible?) to have both an async_context mutex and the
// LWIP one, owned by different TaskHandle_t's, and not end up in a
// deadlock case.  LWIP task may need to call CYW43 functions but if LWIP
// was invoked *by* the CYW43 async_context that mutex is already taken by
// the async_context task and any cyw43 calls from the LWIP task will
// deadlock since they can't re-active the async_context task to resolve
// the mutex.  Ouroboros time...

// Based off of pico-sdk/src/rp2_common/pico_cyw43_driver/cyw43_driver.c,
// Copyright (c) 2022 Raspberry Pi (Trading) Ltd.,
// SPDX-License-Identifier: BSD-3-Clause

#ifdef __FREERTOS

#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"



#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/unique_id.h"
extern "C" {
#include "cyw43.h"
#include "pico/cyw43_driver.h"
}
#include "pico/async_context.h"
#include <lwip_wrap.h>

#ifndef CYW43_GPIO_IRQ_HANDLER_PRIORITY
#define CYW43_GPIO_IRQ_HANDLER_PRIORITY 0x40
#endif

#ifndef CYW43_SLEEP_CHECK_MS
#define CYW43_SLEEP_CHECK_MS 50
#endif

static SemaphoreHandle_t _cyw43_arch_mutex;
static SemaphoreHandle_t _cyw43_irq_called_binary;
static SemaphoreHandle_t _cyw43_sleep_poll_binary;

static void cb_cyw43_do_poll(void *context);
static __callback_req _irqBuffer;

static void cyw43_set_irq_enabled(bool enabled) {
#ifndef NDEBUG
    assert(get_core_num() == 0);
#endif
    gpio_set_irq_enabled(CYW43_PIN_WL_HOST_WAKE, GPIO_IRQ_LEVEL_HIGH, enabled);
}

// GPIO interrupt handler to tell us there's cyw43 has work to do
static void cyw43_gpio_irq_handler() {
#ifndef NDEBUG
    assert(get_core_num() == 0);
#endif
    uint32_t events = gpio_get_irq_event_mask(CYW43_PIN_WL_HOST_WAKE);
    if (events & GPIO_IRQ_LEVEL_HIGH) {
        // As we use a high level interrupt, it will go off forever until it's serviced
        // So disable the interrupt until this is done. It's re-enabled again by CYW43_POST_POLL_HOOK
        // which is called at the end of cyw43_poll_func
        cyw43_set_irq_enabled(false);
        lwip_callback(cb_cyw43_do_poll, nullptr, &_irqBuffer);
    }
}

static uint32_t cyw43_irq_init(__unused void *param) {
#ifndef NDEBUG
    assert(get_core_num() == 0);
#endif
    gpio_add_raw_irq_handler_with_order_priority(CYW43_PIN_WL_HOST_WAKE, cyw43_gpio_irq_handler, CYW43_GPIO_IRQ_HANDLER_PRIORITY);
    cyw43_set_irq_enabled(true);
    irq_set_enabled(IO_IRQ_BANK0, true);
    return 0;
}

extern "C" void __wrap_cyw43_post_poll_hook() {
#ifndef NDEBUG
    assert(get_core_num() == 0);
#endif
    cyw43_set_irq_enabled(true);
}

extern "C" void __wrap_cyw43_schedule_internal_poll_dispatch(__unused void (*func)()) {
    lwip_callback(cb_cyw43_do_poll, nullptr);
}



static int64_t cb_cyw43_sleep_timeout_reached(alarm_id_t id, void *ptr) {
    (void) id;
    (void) ptr;
    static __callback_req _sleepIRQBuffer;
    // This will be in IRQ context, so do a lwip callback.  Only one at a time can be outstanding so this single struct is good enough
    BaseType_t pxHigherPriorityTaskWoken;
    if (xSemaphoreTakeFromISR(_cyw43_sleep_poll_binary, &pxHigherPriorityTaskWoken)) {
        lwip_callback(cb_cyw43_do_poll, nullptr, &_sleepIRQBuffer);
    }
    return 0; // Don't reschedule
}

// By construction, this will only be called from the LWIP thread on core 0
static void cb_cyw43_do_poll(void *context) { //, __unused async_when_pending_worker_t *worker) {
#ifndef NDEBUG
    assert(get_core_num() == 0);
#endif
    cyw43_thread_enter();
    if (cyw43_poll) {
        if (cyw43_sleep > 0) {
            cyw43_sleep--;
        }
        cyw43_poll();
        if (cyw43_sleep) {
            add_alarm_in_ms(CYW43_SLEEP_CHECK_MS, cb_cyw43_sleep_timeout_reached, nullptr, true);
        } else {
            // Nothing to do.  We have 1-shot alarms
        }
    }
    cyw43_thread_exit();
    xSemaphoreGive(_cyw43_irq_called_binary);
    xSemaphoreGive(_cyw43_sleep_poll_binary);
}

extern "C" bool __wrap_cyw43_driver_init(async_context_t *context) {
    assert(get_core_num() == 0);
    _cyw43_arch_mutex = xSemaphoreCreateRecursiveMutex();
    _cyw43_irq_called_binary = xSemaphoreCreateBinary();
    _cyw43_sleep_poll_binary = xSemaphoreCreateBinary();
    xSemaphoreGive(_cyw43_sleep_poll_binary);
    cyw43_init(&cyw43_state);
    cyw43_irq_init(nullptr);
    return true;
}

extern "C" void __wrap_cyw43_driver_deinit(async_context_t *context) {
    panic("unsipported");
}

// Prevent background processing in pensv and access by the other core
// These methods are called in pensv context and on either core
// They can be called recursively
extern "C" void __wrap_cyw43_thread_enter() {
    xSemaphoreTakeRecursive(_cyw43_arch_mutex, portTICK_PERIOD_MS);
}

extern "C" void __wrap_cyw43_thread_exit() {
    xSemaphoreGiveRecursive(_cyw43_arch_mutex);
}

#ifndef NDEBUG
extern "C" void __wrap_cyw43_thread_lock_check() {
    // TODO
}
#endif

extern "C" void __wrap_cyw43_await_background_or_timeout_us(uint32_t timeout_us) {
//    cyw43_set_irq_enabled(true);
    if (__get_current_exception() > 0) {
        vTaskDelay((timeout_us / 1000) / portTICK_PERIOD_MS);
        return;
    }
    // Try and take a binary semaphore that only the IRQ will give or timeout trying
    xSemaphoreTake(_cyw43_irq_called_binary, (timeout_us / 1000) / portTICK_PERIOD_MS);
}

extern "C" void __wrap_cyw43_delay_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

extern "C" void __wrap_cyw43_delay_us(uint32_t us) {
    if (us >= 1000) {
        uint32_t ms = us / 1000;
        vTaskDelay(ms / portTICK_PERIOD_MS);
        us -= ms * 1000;
    }
    delayMicroseconds(us);
}



static int this_cyw43_arch_wifi_connect_bssid_until(const char *ssid, const uint8_t *bssid, const char *pw, uint32_t auth, uint32_t timeout_ms) {
    uint32_t start = millis();
    int err = cyw43_arch_wifi_connect_bssid_async(ssid, bssid, pw, auth);
    if (err) return err;
    int status = CYW43_LINK_UP + 1;
    while(status >= 0 && status != CYW43_LINK_UP) {
        int new_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        // If there was no network, keep trying
        if (new_status == CYW43_LINK_NONET) {
            new_status = CYW43_LINK_JOIN;
            err = cyw43_arch_wifi_connect_bssid_async(ssid, bssid, pw, auth);
            if (err) return err;
        }
        if (new_status != status) {
            status = new_status;
        }
        uint32_t delta = millis() - start;
        if (delta > timeout_ms) {
            return PICO_ERROR_TIMEOUT;
        }
        // Do polling
        //cyw43_arch_poll();
        __wrap_cyw43_await_background_or_timeout_us((timeout_ms - delta) * 1000); //cyw43_arch_wait_for_work_until(until);
    }
    // Turn status into a pico_error_codes, CYW43_LINK_NONET shouldn't happen as we fail with PICO_ERROR_TIMEOUT instead
    assert(status == CYW43_LINK_UP || status == CYW43_LINK_BADAUTH || status == CYW43_LINK_FAIL);
    if (status == CYW43_LINK_UP) {
        return PICO_OK; // success
    } else if (status == CYW43_LINK_BADAUTH) {
        return PICO_ERROR_BADAUTH;
    } else {
        return PICO_ERROR_CONNECT_FAILED;
    }
}
extern "C" int __wrap_cyw43_arch_wifi_connect_bssid_timeout_ms(const char *ssid, const uint8_t *bssid, const char *pw, uint32_t auth, uint32_t timeout_ms) {
    return this_cyw43_arch_wifi_connect_bssid_until(ssid, bssid, pw, auth, timeout_ms); //make_timeout_time_ms(timeout_ms));
}

extern "C" int __wrap_cyw43_arch_wifi_connect_timeout_ms(const char *ssid, const char *pw, uint32_t auth, uint32_t timeout_ms) {
    return __wrap_cyw43_arch_wifi_connect_bssid_timeout_ms(ssid, nullptr, pw, auth, timeout_ms);
}

#endif
