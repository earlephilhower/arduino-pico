/*
    LWIP-on-FreeRTOS Plumbing

    Copyright (c) 2025 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#ifdef __FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <lwip_wrap.h>
#include "freertos-lwip.h"

// The data structure for the LWIP work queue
typedef struct {
    __lwip_op op;
    void *req;
    TaskHandle_t wakeup;
} LWIPWork;

#define LWIP_WORK_ENTRIES 16

#ifndef LWIP_TASK_PRIORITY
#define LWIP_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#endif

// The notify item we'll use to wake the calling process back up
#define TASK_NOTIFY_LWIP_WAKEUP (configTASK_NOTIFICATION_ARRAY_ENTRIES - 1)

static void lwipThread(void *params);
static TaskHandle_t __lwipTask;
static QueueHandle_t __lwipQueue;

void __startLWIPThread() {
    static bool initted = false;
    if (initted) {
        return;
    }
    __lwipQueue = xQueueCreate(LWIP_WORK_ENTRIES, sizeof(LWIPWork));
    if (!__lwipQueue) {
        panic("Unable to allocate LWIP work queue");
    }
    if (pdPASS != xTaskCreate(lwipThread, "LWIP", 1024, 0, LWIP_TASK_PRIORITY, &__lwipTask)) {
        panic("Unable to create LWIP task");
    }
    vTaskCoreAffinitySet(__lwipTask, 1 << 0);
    initted = true;
}

extern "C" void __lwip(__lwip_op op, void *req, bool fromISR) {
    LWIPWork w;
    if (fromISR) {
        w.op = op;
        w.req = req;
        w.wakeup = 0; // Don't try and wake up a task when done, we're not in one!
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (!xQueueSendFromISR(__lwipQueue, &w, &xHigherPriorityTaskWoken)) {
            panic("LWIP task send failed");
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        TaskStatus_t t;
        vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
        w.op = op;
        w.req = req;
        w.wakeup = t.xHandle;
        if (!xQueueSend(__lwipQueue, &w, portMAX_DELAY)) {
            panic("LWIP task send failed");
        }
        ulTaskNotifyTakeIndexed(TASK_NOTIFY_LWIP_WAKEUP, pdTRUE, portMAX_DELAY);
    }
}

void __lwip(std::function<void(void)> *cb) {
    LWIPWork w;

    TaskStatus_t t;
    vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???

    w.op = __function;
    w.req = cb;
    w.wakeup = t.xHandle;
    if (!xQueueSend(__lwipQueue, &w, portMAX_DELAY)) {
        panic("LWIP task send failed");
    }
    ulTaskNotifyTakeIndexed(TASK_NOTIFY_LWIP_WAKEUP, pdTRUE, portMAX_DELAY);
}

extern "C" bool __isLWIPThread() {
    TaskStatus_t t;
    vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
    return t.xHandle == __lwipTask;
}

static void lwipThread(void *params) {
    (void) params;
    LWIPWork w;
    assert(__isLWIPThread());
    unsigned int scd = 100 / portTICK_PERIOD_MS;

    lwip_init(); // Will call our wrapper and set up the RNG

    while (true) {
        auto ret = xQueueReceive(__lwipQueue, &w, scd);
        if (ret) {
            switch (w.op) {
            case __lwip_init: {
                __real_lwip_init();
                break;
            }

#include "../lwip-wrappers/wrap_pbuf_cases.inc"
#include "../lwip-wrappers/wrap_raw_cases.inc"
#include "../lwip-wrappers/wrap_tcp_cases.inc"
#include "../lwip-wrappers/wrap_udp_cases.inc"
#include "../lwip-wrappers/wrap_netif_cases.inc"
#include "../lwip-wrappers/wrap_dns_cases.inc"
#include "../lwip-wrappers/wrap_dhcp_cases.inc"
#include "../lwip-wrappers/wrap_igmp_cases.inc"
#include "../lwip-wrappers/wrap_mld6_cases.inc"
#include "../lwip-wrappers/wrap_mdns_cases.inc"
#include "../lwip-wrappers/wrap_sntp_cases.inc"
#include "../lwip-wrappers/wrap_ethernet_cases.inc"

            case __sys_check_timeouts: {
                __real_sys_check_timeouts();
                break;
            }
            case __sys_timeouts_sleeptime: {
                __sys_timeouts_sleeptime_req *r = (__sys_timeouts_sleeptime_req *)w.req;
                *(r->ret) = __real_sys_timeouts_sleeptime();
                break;
            }

#if defined(PICO_CYW43_SUPPORTED)
            case __cyw43_wifi_join: {
                __cyw43_wifi_join_req *r = (__cyw43_wifi_join_req *)w.req;
                *(r->ret) = __real_cyw43_wifi_join(r->self, r->ssid_len, r->ssid, r->key_len, r->key, r->auth_type, r->bssid, r->channel);
                break;
            }
            case __cyw43_wifi_leave: {
                __cyw43_wifi_leave_req *r = (__cyw43_wifi_leave_req*)w.req;
                *(r->ret) = __real_cyw43_wifi_leave(r->self, r->itf);
                break;
            }
            case __cyw43_ioctl: {
                __cyw43_ioctl_req *r = (__cyw43_ioctl_req *)w.req;
                *(r->ret) = __real_cyw43_ioctl(r->self, r->cmd, r->len, r->buf, r->iface);
                break;
            }
            case __cyw43_wifi_update_multicast_filter: {
                __cyw43_wifi_update_multicast_filter_req *r = (__cyw43_wifi_update_multicast_filter_req *)w.req;
                *(r->ret) = __real_cyw43_wifi_update_multicast_filter(r->self, r->addr, r->add);
                break;
            }
#endif
            case __callback: {
                __callback_req *r = (__callback_req *)w.req;
                r->cb(r->cbData);
                break;
            }

            case __function: {
                std::function<void(void)> *cb = (std::function<void(void)> *)w.req;
                (*cb)();
                break;
            }

            default: {
                // Any new unimplemented calls = ERROR!!!
                panic("Unimplemented LWIP thread action");
                break;
            }
            }
            // Work done, return value set, just tickle the calling task
            if (w.wakeup) {
                xTaskNotifyGiveIndexed(w.wakeup, TASK_NOTIFY_LWIP_WAKEUP);
            }
        } else {
            // No work received, do periodic processing
            __real_sys_check_timeouts();
            // When should we wake up next to redo timeouts?
            scd = sys_timeouts_sleeptime();
            if (scd == SYS_TIMEOUTS_SLEEPTIME_INFINITE) {
                scd = portMAX_DELAY / portTICK_PERIOD_MS;
            }
        }
    }
}

#endif
