#pragma once

#include <stdint.h>

// HID Gamepad Protocol Report.
typedef struct __attribute__((packed)) {
    int16_t x;         ///< Delta x  movement of left analog-stick
    int16_t y;         ///< Delta y  movement of left analog-stick
    int16_t z;         ///< Delta z  movement of right analog-joystick
    int16_t rz;        ///< Delta Rz movement of right analog-joystick
    int16_t rx;        ///< Delta Rx movement of analog left trigger
    int16_t ry;        ///< Delta Ry movement of analog right trigger
    uint8_t hat;       ///< Buttons mask for currently pressed buttons in the DPad/hat
    uint32_t buttons;  ///< Buttons mask for currently pressed buttons
} hid_gamepad16_report_t;
