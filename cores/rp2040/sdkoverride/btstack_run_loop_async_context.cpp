// Hacked by EFP3 to with FreeRTOS native, no async context.

#ifdef __FREERTOS
/*
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <Arduino.h>
#include <lwip_wrap.h>

#include "pico/btstack_run_loop_async_context.h"
#include "hardware/sync.h"

//static async_context_t *btstack_async_context;
//static async_at_time_worker_t btstack_timeout_worker;
//static async_when_pending_worker_t btstack_processing_worker;
static void btstack_timeout_reached(async_context_t *context, async_at_time_worker_t *worker);
static void btstack_work_pending(async_context_t *context, async_when_pending_worker_t *worker);
static volatile bool run_loop_exit;

static void do_btstack_work_pending(void *data) {
    btstack_work_pending(NULL, NULL);
}

static void btstack_run_loop_async_context_init(void) {
    btstack_run_loop_base_init();
    //btstack_timeout_worker.do_work = btstack_timeout_reached;
//    btstack_processing_worker.do_work = btstack_work_pending;
//    async_context_add_when_pending_worker(btstack_async_context, &btstack_processing_worker);
}

static void btstack_run_loop_async_context_add_data_source(btstack_data_source_t * data_source) {
    cyw43_thread_enter();
    btstack_run_loop_base_add_data_source(data_source);
    cyw43_thread_exit();
}

static bool btstack_run_loop_async_context_remove_data_source(btstack_data_source_t * data_source) {
    cyw43_thread_enter();
    bool rc = btstack_run_loop_base_remove_data_source(data_source);
    cyw43_thread_exit();
    return rc;
}

static void btstack_run_loop_async_context_enable_data_source_callbacks(btstack_data_source_t * data_source, uint16_t callbacks) {
    cyw43_thread_enter();
    btstack_run_loop_base_enable_data_source_callbacks(data_source, callbacks);
    cyw43_thread_exit();
}

static void btstack_run_loop_async_context_disable_data_source_callbacks(btstack_data_source_t * data_source, uint16_t callbacks) {
    cyw43_thread_enter();
    btstack_run_loop_base_disable_data_source_callbacks(data_source, callbacks);
    cyw43_thread_exit();
}

static void btstack_run_loop_async_context_set_timer(btstack_timer_source_t *ts, uint32_t timeout_in_ms){
    cyw43_thread_enter();
    ts->timeout = to_ms_since_boot(get_absolute_time()) + timeout_in_ms + 1;
    lwip_callback(do_btstack_work_pending, NULL);
    cyw43_thread_exit();
}

static void btstack_run_loop_async_context_add_timer(btstack_timer_source_t *timer) {
    cyw43_thread_enter();
    btstack_run_loop_base_add_timer(timer);
    lwip_callback(do_btstack_work_pending, NULL);
    cyw43_thread_exit();
}

static bool btstack_run_loop_async_context_remove_timer(btstack_timer_source_t *timer) {
    cyw43_thread_enter();
    bool rc = btstack_run_loop_base_remove_timer(timer);
    cyw43_thread_exit();
    return rc;
}

static void btstack_run_loop_async_context_dump_timer(void){
    cyw43_thread_enter();
    btstack_run_loop_base_dump_timer();
    cyw43_thread_exit();
}

static uint32_t btstack_run_loop_async_context_get_time_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

static void btstack_run_loop_async_context_execute(void)
{
    run_loop_exit = false;
    while (!run_loop_exit) {
//        async_context_poll(btstack_async_context);
//        async_context_wait_for_work_until(btstack_async_context, at_the_end_of_time);
        delay(1);
    }
}

static void btstack_run_loop_async_context_trigger_exit(void)
{
    run_loop_exit = true;
}

static void btstack_run_loop_async_context_execute_on_main_thread(btstack_context_callback_registration_t *callback_registration)
{
    cyw43_thread_enter();
    btstack_run_loop_base_add_callback(callback_registration);
    lwip_callback(do_btstack_work_pending, NULL);
    cyw43_thread_exit();
}

static void btstack_run_loop_async_context_poll_data_sources_from_irq(void)
{
    lwip_callback(do_btstack_work_pending, NULL);
}

static const btstack_run_loop_t btstack_run_loop_async_context = {
    &btstack_run_loop_async_context_init,
    &btstack_run_loop_async_context_add_data_source,
    &btstack_run_loop_async_context_remove_data_source,
    &btstack_run_loop_async_context_enable_data_source_callbacks,
    &btstack_run_loop_async_context_disable_data_source_callbacks,
    &btstack_run_loop_async_context_set_timer,
    &btstack_run_loop_async_context_add_timer,
    &btstack_run_loop_async_context_remove_timer,
    &btstack_run_loop_async_context_execute,
    &btstack_run_loop_async_context_dump_timer,
    &btstack_run_loop_async_context_get_time_ms,
    &btstack_run_loop_async_context_poll_data_sources_from_irq,
    &btstack_run_loop_async_context_execute_on_main_thread,
    &btstack_run_loop_async_context_trigger_exit,
};

extern "C" const btstack_run_loop_t *__wrap_btstack_run_loop_async_context_get_instance(async_context_t *async_context)
{
    return &btstack_run_loop_async_context;
}

static void btstack_timeout_reached(__unused async_context_t *context, __unused async_at_time_worker_t *worker) {
    // simply wakeup worker
    lwip_callback(do_btstack_work_pending, NULL);
}
static alarm_id_t _timeout = -1;
static int64_t cb_btstack_timeout_worker(alarm_id_t id, void *user_data) {
    static __callback_req _timeoutIRQBuffer;
    // This will be in IRQ context, so do a lwip callback.  Only one at a time can be outstanding so this single struct is good enough
    _timeout = -1;
    lwip_callback(do_btstack_work_pending, NULL, &_timeoutIRQBuffer);
    return 0; // Don't reschedule
}

static void btstack_work_pending(__unused async_context_t *context, __unused async_when_pending_worker_t *worker) {
    // poll data sources
    btstack_run_loop_base_poll_data_sources();

    // execute callbacks
    btstack_run_loop_base_execute_callbacks();

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // process timers
    btstack_run_loop_base_process_timers(now);
    now = to_ms_since_boot(get_absolute_time());
    int ms = btstack_run_loop_base_get_time_until_timeout(now);
    if (ms == -1) {
        if (_timeout != -1) {
            cancel_alarm(_timeout);
            _timeout = -1;
        //async_context_remove_at_time_worker(btstack_async_context, &btstack_timeout_worker);
        }
    } else {
        _timeout = add_alarm_in_ms(ms, cb_btstack_timeout_worker, NULL, true);
    }
}
#endif
