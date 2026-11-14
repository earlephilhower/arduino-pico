// Part of the WM8960USB example. Released to the public domain 2026 by K7MDL2
#include "composite_config.h"
#if WM8960USB_SUPPORTED

#include "composite_config.h"

// Only CDC0 may reach SerialUSB's line-state callbacks (including 1200-baud reset).
#define tud_cdc_line_coding_cb composite_line_coding_cb
#define tud_cdc_line_state_cb composite_line_state_cb
#include <class/cdc/cdc_device.c>
#undef tud_cdc_line_coding_cb
#undef tud_cdc_line_state_cb

extern void tud_cdc_line_coding_cb(uint8_t, cdc_line_coding_t const *);
extern void tud_cdc_line_state_cb(uint8_t, bool, bool);

void composite_line_coding_cb(uint8_t instance, cdc_line_coding_t const *coding) {
    if (instance == 0) {
        tud_cdc_line_coding_cb(instance, coding);
    }
}

void composite_line_state_cb(uint8_t instance, bool dtr, bool rts) {
    if (instance == 0) {
        tud_cdc_line_state_cb(instance, dtr, rts);
    }
}

#endif // WM8960USB_SUPPORTED
