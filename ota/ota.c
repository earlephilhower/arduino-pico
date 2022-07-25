#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/irq.h"
#include "hardware/regs/m0plus.h"
#include "hardware/platform_defs.h"
#include "hardware/structs/scb.h"
#include "hardware/claim.h"

#include "pico/mutex.h"
#include "pico/assert.h"

#pragma GCC push_options
#pragma GCC optimize("O0")

// OTA flasher
int main(unsigned char **a, int b) {
    (void) a;
    (void) b;
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    for (int i=0; i<10; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }

    // Reset the interrupt/etc. vectors to the real app
    uint32_t *vecsrc = (uint32_t*)0x10004000;
    uint32_t *vecdst = (uint32_t*)0x20000000;
    for (int i=0; i<48; i++) {
        *vecdst++ = *vecsrc++;
    }

    // Jump to it
    register uint32_t* sp asm("sp");
    register uint32_t _sp = *(uint32_t *)0x10004000;
    register void (*fcn)(void) = *(uint32_t *)0x10004004;
    sp = _sp;
    fcn();

    // Should never get here!
    return 0;
}

// Clear out some unwanted extra code
int __wrap_atexit(void (*function)(void)) {
    (void) function;
    return 0;
}

// Clear out some unwanted extra code
void __wrap_exit(int status) {
    (void) status;
    while (1) continue;
}

#pragma GCC pop_options
