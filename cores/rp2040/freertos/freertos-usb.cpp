/*
    FreeRTOS USB task

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

#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Arduino Core includes */
#include <Arduino.h>
#include <RP2040USB.h>
#include "tusb.h"

/* Raspberry PI Pico includes */
#include <pico.h>
#include <pico/time.h>

#include <_freertos.h>

#include "freertos-usb.h"



extern mutex_t __usb_mutex;
static TaskHandle_t __usbTask;
static void __usb(void *param);

volatile bool __usbInitted = false;

static void __usb(void *param) {
    (void) param;

    tusb_init();

    Serial.begin(115200);

    __usbInitted = true;

    while (true) {
        BaseType_t ss = xTaskGetSchedulerState();
        if (ss != taskSCHEDULER_SUSPENDED) {
            auto m = __get_freertos_mutex_for_ptr(&__usb_mutex);
            if (xSemaphoreTake(m, 0)) {
                tud_task();
                xSemaphoreGive(m);
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

extern void __SetupDescHIDReport();
extern void __SetupUSBDescriptor();

void __USBStart() {
    mutex_init(&__usb_mutex);

    __SetupDescHIDReport();
    __SetupUSBDescriptor();

    // Make high prio and locked to core 0
    xTaskCreate(__usb, "USB", 256, 0, configMAX_PRIORITIES - 2, &__usbTask);
    vTaskCoreAffinitySet(__usbTask, 1 << 0);
}

#endif
