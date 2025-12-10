// Dummy shim to be overridden by lwip_usb_ncm
#include <Arduino.h>
#include "tusb_option.h"

#if (CFG_TUD_ENABLED && CFG_TUD_NCM)

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "device/usbd.h"
#include "device/usbd_pvt.h"

#include "../../../pico-sdk/lib/tinyusb/src/class/net/ncm.h"
#include "../../../pico-sdk/lib/tinyusb/src/class/net/net_device.h"


extern bool tud_network_can_xmit(uint16_t size) __attribute((weak));
bool tud_network_can_xmit(uint16_t size) {
    (void) size;
    return false;
}

extern void tud_network_xmit(void *ref, uint16_t arg) __attribute((weak));
void tud_network_xmit(void *ref, uint16_t arg) {
    (void) ref;
    (void) arg;
    return;
}

extern void tud_network_recv_renew(void) __attribute((weak));
void tud_network_recv_renew(void) {
}

extern void netd_init(void) __attribute((weak));
void netd_init(void) {
}

extern bool netd_deinit(void) __attribute((weak));
bool netd_deinit(void) {
    return true;
}

extern void netd_reset(uint8_t rhport) __attribute((weak));
void netd_reset(uint8_t rhport) {
    (void) rhport;
}

extern uint16_t netd_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) __attribute((weak));
uint16_t netd_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    (void) rhport;
    (void) itf_desc;
    (void) max_len;
    return 0;
}

extern bool netd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) __attribute((weak));
bool netd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    (void) rhport;
    (void) ep_addr;
    (void) result;
    (void) xferred_bytes;
    return false;
}

extern bool netd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) __attribute((weak));
bool netd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    (void) rhport;
    (void) stage;
    (void) request;
    return false;
}

#endif // ( CFG_TUD_ENABLED && CFG_TUD_NCM )
