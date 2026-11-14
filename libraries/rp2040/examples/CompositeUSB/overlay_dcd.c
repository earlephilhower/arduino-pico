// Part of the CompositeUSB example. Released to the public domain 2026 by K7MDL2
// TinyUSB #2937 backport, as used by RP2350-SDR-CW-Interface.
// Includes the installed MIT-licensed TinyUSB driver; see its license header.
#include "composite_config.h"
#define dcd_edpt_iso_activate composite_original_iso_activate
#include <portable/raspberrypi/rp2040/dcd_rp2040.c>
#undef dcd_edpt_iso_activate

static void abort_iso_transfer(struct hw_endpoint *ep) {
    const uint8_t direction = tu_edpt_dir(ep->ep_addr);
    const uint8_t endpoint = tu_edpt_number(ep->ep_addr);
    const uint32_t mask = TU_BIT((endpoint << 1) | (direction ? 0 : 1));
    if (rp2040_chip_version() >= 2) {
        usb_hw_set->abort = mask;
        while ((usb_hw->abort_done & mask) != mask) {}
    }
    uint32_t control = USB_BUF_CTRL_SEL;
    if (ep->next_pid) {
        control |= USB_BUF_CTRL_DATA1_PID;
    }
    _hw_endpoint_buffer_control_set_value32(ep, control);
    hw_endpoint_reset_transfer(ep);
    if (rp2040_chip_version() >= 2) {
        usb_hw_clear->abort_done = mask;
        usb_hw_clear->abort = mask;
    }
}

bool dcd_edpt_iso_activate(uint8_t rhport, tusb_desc_endpoint_t const *descriptor) {
    (void)rhport;
    struct hw_endpoint *ep = hw_endpoint_get_by_addr(descriptor->bEndpointAddress);
    TU_ASSERT(ep->hw_data_buf != NULL);
    if (ep->active) {
        abort_iso_transfer(ep);
    }
    ep->wMaxPacketSize = descriptor->wMaxPacketSize;
    hw_endpoint_enable(ep);
    return true;
}
