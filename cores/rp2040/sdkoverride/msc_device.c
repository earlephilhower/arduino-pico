#include <stdint.h>
#include <tusb.h>

extern void mscd_init() __attribute__((weak));
void mscd_init() {
}

extern bool mscd_deinit() __attribute__((weak));
bool mscd_deinit() {
    return true;
}

extern void mscd_reset(uint8_t rhport) __attribute__((weak));
void mscd_reset(uint8_t rhport) {
    (void) rhport;
}

extern uint16_t mscd_open(uint8_t rhport, tusb_desc_interface_t const * itf_desc, uint16_t max_len) __attribute__((weak));
uint16_t mscd_open(uint8_t rhport, tusb_desc_interface_t const * itf_desc, uint16_t max_len) {
    (void) rhport;
    (void) itf_desc;
    (void) max_len;
    return 0;
}

extern bool mscd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) __attribute__((weak));
bool mscd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    (void) rhport;
    (void) stage;
    (void) request;
    return false;
}

bool mscd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes) __attribute__((weak));
bool mscd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes) {
    (void) rhport;
    (void) ep_addr;
    (void) event;
    (void) xferred_bytes;
    return false;
}
