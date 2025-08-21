#if defined(__FREERTOS)
// Taken from SDK because we need to remove the !NO_SYS check
#define PICO_CYW43_ARCH_FREERTOS 1
/*
    Copyright (c) 2022 Raspberry Pi (Trading) Ltd.

    SPDX-License-Identifier: BSD-3-Clause
*/

#if PICO_CYW43_ARCH_FREERTOS

#include "pico/cyw43_arch.h"
#include "pico/cyw43_driver.h"

#if CYW43_LWIP
#include "pico/lwip_freertos.h"
#include <lwip/tcpip.h>
#endif

#if CYW43_ENABLE_BLUETOOTH
#include "pico/btstack_cyw43.h"
#endif


int cyw43_arch_init(void) {
    async_context_t *context = NULL;
    cyw43_arch_set_async_context(context);

    bool ok = cyw43_driver_init(context);
#if CYW43_LWIP
    ok &= lwip_freertos_init(context);
#endif
#if CYW43_ENABLE_BLUETOOTH
    ok &= btstack_cyw43_init(context);
#endif
    if (!ok) {
        cyw43_arch_deinit();
        return PICO_ERROR_GENERIC;
    } else {
        return 0;
    }
}

void cyw43_arch_deinit(void) {
    panic("Unsupported");
}

#endif

#endif
