#include <Arduino.h>

SerialUSB Serial;
extern "C" {
#include "tusb.h"
#include "pico/time.h"
#include "pico/binary_info.h"
#include "hardware/irq.h"
#include "pico/mutex.h"
#include "hardware/watchdog.h"
}
#define PICO_STDIO_USB_TASK_INTERVAL_US 1000
#define PICO_STDIO_USB_LOW_PRIORITY_IRQ 31

#define USBD_VID (0x2E8A) // Raspberry Pi
#define USBD_PID (0x000a) // Raspberry Pi Pico SDK CDC

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)
#define USBD_MAX_POWER_MA (250)

#define USBD_ITF_CDC (0) // needs 2 interfaces
#define USBD_ITF_MAX (2)

#define USBD_CDC_EP_CMD (0x81)
#define USBD_CDC_EP_OUT (0x02)
#define USBD_CDC_EP_IN (0x82)
#define USBD_CDC_CMD_MAX_SIZE (8)
#define USBD_CDC_IN_OUT_MAX_SIZE (64)

#define USBD_STR_0 (0x00)
#define USBD_STR_MANUF (0x01)
#define USBD_STR_PRODUCT (0x02)
#define USBD_STR_SERIAL (0x03)
#define USBD_STR_CDC (0x04)

// Note: descriptors returned from callbacks must exist long enough for transfer to complete

static const tusb_desc_device_t usbd_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUF,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA),

    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_CDC, USBD_CDC_EP_CMD,
        USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),
};

static const char *const usbd_desc_str[] = {
    [USBD_STR_0] = "",
    [USBD_STR_MANUF] = "Raspberry Pi",
    [USBD_STR_PRODUCT] = "PicoArduino",
    [USBD_STR_SERIAL] = "000000000000", // TODO
    [USBD_STR_CDC] = "Board CDC",
};

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return usbd_desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    #define DESC_STR_MAX (20)
    static uint16_t desc_str[DESC_STR_MAX];

    uint8_t len;
    if (index == 0) {
        desc_str[1] = 0x0409; // supported language is English
        len = 1;
    } else {
        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
            return NULL;
        }
        const char *str = usbd_desc_str[index];
        for (len = 0; len < DESC_STR_MAX - 1 && str[len]; ++len) {
            desc_str[1 + len] = str[len];
        }
    }

    // first byte is length (including header), second byte is string type
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

    return desc_str;
}

static mutex_t usb_mutex;

static void low_priority_worker_irq() {
    // if the mutex is already owned, then we are in user code
    // in this file which will do a tud_task itself, so we'll just do nothing
    // until the next tick; we won't starve
    if (mutex_try_enter(&usb_mutex, NULL)) {
        tud_task();
        mutex_exit(&usb_mutex);
    }
}

static int64_t timer_task(__unused alarm_id_t id, __unused void *user_data) {
    irq_set_pending(PICO_STDIO_USB_LOW_PRIORITY_IRQ);
    return PICO_STDIO_USB_TASK_INTERVAL_US;
}

void SerialUSB::begin(int baud) {
    (void) baud; //ignored

    tusb_init();

    irq_set_exclusive_handler(PICO_STDIO_USB_LOW_PRIORITY_IRQ, low_priority_worker_irq);
    irq_set_enabled(PICO_STDIO_USB_LOW_PRIORITY_IRQ, true);

    mutex_init(&usb_mutex);
    add_alarm_in_us(PICO_STDIO_USB_TASK_INTERVAL_US, timer_task, NULL, true);
}

void SerialUSB::end() {
    // TODO
}

int SerialUSB::peek() {
    uint8_t c;
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return -1; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    auto ret = tud_cdc_peek(0, &c) ? (int) c : -1;
    mutex_exit(&usb_mutex);
    return ret;
}

int SerialUSB::read() {
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return -1; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    if (tud_cdc_connected() && tud_cdc_available()) {
        int ch = tud_cdc_read_char();
        mutex_exit(&usb_mutex);
        return ch;
    }
    mutex_exit(&usb_mutex);
    return -1;
}
int SerialUSB::available() {
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return 0; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    auto ret = tud_cdc_available();
    mutex_exit(&usb_mutex);
    return ret;
}
int SerialUSB::availableForWrite() {
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return 0; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    auto ret = tud_cdc_write_available();
    mutex_exit(&usb_mutex);
    return ret;
}
void SerialUSB::flush() {
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    tud_cdc_write_flush();
    mutex_exit(&usb_mutex);
}
size_t SerialUSB::write(uint8_t c) {
    return write(&c, 1);
}
size_t SerialUSB::write(const uint8_t *p, size_t len) {
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return 0; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    size_t remain = len;
    while (remain && tud_cdc_connected()) {
        size_t cnt = tud_cdc_write(p, remain);
        p += cnt;
        remain -= cnt;
        tud_task();
        tud_cdc_write_flush();
    }
    mutex_exit(&usb_mutex);
    return len - remain;
}
SerialUSB::operator bool() {
    uint32_t owner;
    if (!mutex_try_enter(&usb_mutex, &owner)) {
        if (owner == get_core_num()) return -1; // would deadlock otherwise
        mutex_enter_blocking(&usb_mutex);
    }
    tud_task();
    auto ret = tud_cdc_connected();
    mutex_exit(&usb_mutex);
    return ret;
}


static bool _dtr = false;
static bool _rts = false;
static int _bps = 115200;
static void CheckSerialReset() {
    if ((_bps == 1200) && (!_dtr)) {
        watchdog_enable(100, 1);
	while (1); // WDT will fire here
    }
}


extern "C" void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    _dtr = dtr ? true : false;
    _rts = rts ? true : false;
    CheckSerialReset();
}

extern "C" void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    _bps = p_line_coding->bit_rate;
    CheckSerialReset();
}

