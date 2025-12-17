#include <stdint.h>
#include <tusb.h>

extern void midid_init() __attribute__((weak));
void midid_init() {
}

extern bool midid_deinit() __attribute__((weak));
bool midid_deinit() {
    return true;
}

extern void midid_reset(uint8_t rhport) __attribute__((weak));
void midid_reset(uint8_t rhport) {
    (void) rhport;
}

extern uint16_t midid_open(uint8_t rhport, const tusb_desc_interface_t* desc_itf, uint16_t max_len) __attribute__((weak));
uint16_t midid_open(uint8_t rhport, const tusb_desc_interface_t* desc_itf, uint16_t max_len) {
    (void) rhport;
    (void) desc_itf;
    (void) max_len;
    return 0;
}

extern bool midid_control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t* request) __attribute__((weak));
bool midid_control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t* request) {
    (void) rhport;
    (void) stage;
    (void) request;
    return false;
}

extern bool midid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) __attribute__((weak));
bool midid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    (void) rhport;
    (void) ep_addr;
    (void) result;
    (void) xferred_bytes;
    return false;
}
