// Part of the CompositeUSB example. Released to the public domain 2026 by K7MDL2
#include "composite_config.h"
#include <device/usbd.h>
#include <device/usbd_pvt.h>
#include <class/audio/audio_device.c>

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *count) {
    static usbd_class_driver_t const driver = {
        "AUDIO", audiod_init, audiod_deinit, audiod_reset, audiod_open,
        audiod_control_xfer_cb, audiod_xfer_cb, audiod_sof_isr,
    };
    *count = 1;
    return &driver;
}
