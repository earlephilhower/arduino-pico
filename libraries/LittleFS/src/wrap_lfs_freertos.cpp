/*
    LFS-on-FreeRTOS Plumbing
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

#ifdef __FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "./wrap_lfs.h"
#include "./wrap_lfs_freertos.h"

// The data structure for the work queue
typedef struct {
    __lfs_op op;
    void *req;
    TaskHandle_t wakeup;
} LFSWork;

#define LFS_WORK_ENTRIES 16

#ifndef LFS_TASK_PRIORITY
#define LFS_TASK_PRIORITY (configMAX_PRIORITIES - 3)
#endif

// The notify item we'll use to wake the calling process back up
#define TASK_NOTIFY_LFS_WAKEUP (configTASK_NOTIFICATION_ARRAY_ENTRIES - 1)

static void lfsThread(void *params);
static TaskHandle_t __lfsTask;
static QueueHandle_t __lfsQueue;

void __initLFSMutex() {
    // No real mutex, just a queue
    __lfsQueue = xQueueCreate(LFS_WORK_ENTRIES, sizeof(LFSWork));
    if (!__lfsQueue) {
        panic("Unable to allocate LFS work queue");
    }
    if (pdPASS != xTaskCreate(lfsThread, "LFS", 1024, 0, LFS_TASK_PRIORITY, &__lfsTask)) {
        panic("Unable to create LFS task");
    }
}

extern "C" void __lfs(__lfs_op op, void *req, bool fromISR) {
    (void) req;
    LFSWork w;
    if (fromISR) {
        panic("LFS from an ISR not supported");
    } else {
        TaskStatus_t t;
        vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
        w.op = op;
        w.req = req;
        w.wakeup = t.xHandle;
        if (!xQueueSend(__lfsQueue, &w, portMAX_DELAY)) {
            panic("LFS task send failed");
        }
        ulTaskNotifyTakeIndexed(TASK_NOTIFY_LFS_WAKEUP, pdTRUE, portMAX_DELAY);
    }
}

extern "C" bool __isLFSThread() {
    TaskStatus_t t;
    vTaskGetInfo(nullptr, &t, pdFALSE, eInvalid); // TODO - can we speed this up???
    return t.xHandle == __lfsTask;
}

static void lfsThread(void *params) {
    (void) params;
    LFSWork w;
    assert(__isLFSThread());

    while (true) {
        auto ret = xQueueReceive(__lfsQueue, &w, portMAX_DELAY);
        if (ret) {
            switch (w.op) {
#include "./wrap_lfs_cases.inc"
            default: {
                // Any new unimplemented calls = ERROR!!!
                panic("Unimplemented LFS thread action");
                break;
            }
            }
            // Work done, return value set, just tickle the calling task
            if (w.wakeup) {
                xTaskNotifyGiveIndexed(w.wakeup, TASK_NOTIFY_LFS_WAKEUP);
            }
        } else {
            // Whelp, should never hit here
            panic("No LFS work received");
        }
    }
}

#endif
