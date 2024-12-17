#include "HID_Bluetooth.h"
#include <sdkoverride/tusb_gamepad16.h>

//setup the report map.
//more generic function to be used with BLE & BT Classis
void __SetupHIDreportmap(void (*WeakMouse)(), void (*WeakKeyboard)(), void (*WeakJoystick)(), bool absMouse, uint16_t *report_size, uint8_t **reportmap) {
    //allocate memory for the HID report descriptors. We don't use them, but need the size here.
    uint8_t desc_hid_report_mouse[] = { TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(1)) };
    uint8_t desc_hid_report_absmouse[] = { TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(1)) };
    uint8_t desc_hid_report_joystick[] = { TUD_HID_REPORT_DESC_GAMEPAD16(HID_REPORT_ID(1)) };
    uint8_t desc_hid_report_keyboard[] = { TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)), TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2)) };
    int size = 0;

    //enable to debug the individual report maps
#if 0
    Serial.printf("Report mouse: %d bytes\n", sizeof(desc_hid_report_mouse));
    for (uint16_t i = 0; i < sizeof(desc_hid_report_mouse); i++) {
        Serial.print(desc_hid_report_mouse[i], HEX);
        Serial.print(" ");
        if (i % 4 == 3) {
            Serial.print("\n");
        }
    }
    Serial.printf("Report absmouse: %d bytes\n", sizeof(desc_hid_report_absmouse));
    for (uint16_t i = 0; i < sizeof(desc_hid_report_absmouse); i++) {
        Serial.print(desc_hid_report_absmouse[i], HEX);
        Serial.print(" ");
        if (i % 4 == 3) {
            Serial.print("\n");
        }
    }
    Serial.printf("Report kbd: %d bytes\n", sizeof(desc_hid_report_keyboard));
    for (uint16_t i = 0; i < sizeof(desc_hid_report_keyboard); i++) {
        Serial.print(desc_hid_report_keyboard[i], HEX);
        Serial.print(" ");
        if (i % 4 == 3) {
            Serial.print("\n");
        }
    }
    Serial.printf("Report joystick: %d bytes\n", sizeof(desc_hid_report_joystick));
    for (uint16_t i = 0; i < sizeof(desc_hid_report_joystick); i++) {
        Serial.print(desc_hid_report_joystick[i], HEX);
        Serial.print(" ");
        if (i % 4 == 3) {
            Serial.print("\n");
        }
    }
#endif

    //accumulate the size of all used HID report descriptors
    if (WeakKeyboard) {
        size += sizeof(desc_hid_report_keyboard);
    }
    if (WeakMouse && absMouse == false) {
        size += sizeof(desc_hid_report_mouse);
    } else if (WeakMouse && absMouse == true) {
        size += sizeof(desc_hid_report_absmouse);
    }
    if (WeakJoystick) {
        size += sizeof(desc_hid_report_joystick);
    }

    //no HID used at all
    if (size == 0) {
        *report_size = 0;
        return;
    }

    //allocate the "real" HID report descriptor
    *reportmap = (uint8_t *)malloc(size);
    if (*reportmap) {
        *report_size = size;

        //now copy the descriptors

        //1.) keyboard descriptor, if requested
        if (WeakKeyboard) {
            memcpy(*reportmap, desc_hid_report_keyboard, sizeof(desc_hid_report_keyboard));
        }

        //2.) mouse descriptor, if necessary. Additional offset & new array is necessary if there is a keyboard.
        if (WeakMouse && absMouse == false) {
            //determine if we need an offset (USB keyboard is installed)
            if (WeakKeyboard) {
                uint8_t desc_local[] = { TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(3)) };
                memcpy(*reportmap + sizeof(desc_hid_report_keyboard), desc_local, sizeof(desc_local));
            } else {
                memcpy(*reportmap, desc_hid_report_mouse, sizeof(desc_hid_report_mouse));
            }
        } else if (WeakMouse && absMouse == true) {
            //determine if we need an offset (USB keyboard is installed)
            if (WeakKeyboard) {
                uint8_t desc_local[] = { TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(3)) };
                memcpy(*reportmap + sizeof(desc_hid_report_keyboard), desc_local, sizeof(desc_local));
            } else {
                memcpy(*reportmap, desc_hid_report_absmouse, sizeof(desc_hid_report_absmouse));
            }
        }

        //3.) joystick descriptor. 2 additional checks are necessary for mouse and/or keyboard
        if (WeakJoystick) {
            uint8_t reportid = 1;
            int offset = 0;
            if (WeakKeyboard) {
                reportid += 2;
                offset += sizeof(desc_hid_report_keyboard);
            }
            if (WeakMouse && absMouse == false) {
                reportid++;
                offset += sizeof(desc_hid_report_mouse);
            } else if (WeakMouse && absMouse == true) {
                reportid++;
                offset += sizeof(desc_hid_report_absmouse);
            }
            uint8_t desc_local[] = { TUD_HID_REPORT_DESC_GAMEPAD16(HID_REPORT_ID(reportid)) };
            memcpy(*reportmap + offset, desc_local, sizeof(desc_local));
        }

        //enable for debugging the final report map
#if 0
        Serial.begin(115200);
        Serial.printf("Final map: %d bytes\n", size);
        for (uint16_t i = 0; i < size; i++) {
            Serial.print(*reportmap[i], HEX);
            Serial.print(" ");
            if (i % 4 == 3) {
                Serial.print("\n");
            }
        }
#endif
    } else {
        Serial.println("No report map pointer provided!");
    }
}


//get Class of Device number for starting HID, type depends on activated libraries
uint16_t __BTGetCOD() {
    //mouse only
    if (__BTInstallMouse && !__BTInstallKeyboard && !__BTInstallJoystick) {
        return 0x2580;
    }
    //keyboard only
    if (__BTInstallKeyboard && !__BTInstallMouse && !__BTInstallJoystick) {
        return 0x2540;
    }
    //joystick only
    if (__BTInstallJoystick && !__BTInstallKeyboard && !__BTInstallMouse) {
        return 0x2508;
    }
    //any other combination will return "combo device"
    return 0x25C0;
}

//get Class of Device number for starting HID, type depends on activated libraries
uint16_t __BLEGetAppearance() {
    //mouse only
    if (__BLEInstallMouse && !__BLEInstallKeyboard && !__BLEInstallJoystick) {
        return 0x03C2;
    }
    //keyboard only
    if (__BLEInstallKeyboard && !__BLEInstallMouse && !__BLEInstallJoystick) {
        return 0x03C1;
    }
    //joystick only
    if (__BLEInstallJoystick && !__BLEInstallMouse && !__BLEInstallKeyboard) {
        return 0x03C4;
    }
    //any other combination will return "generic HID"
    return 0x03C0;
}

//keyboard report id is always 1 (compatibility with iOS)
int __BTGetKeyboardReportID() {
    return 1;
}

//
int __BTGetMouseReportID() {
    return __BTInstallKeyboard ? 3 : 1;
}

int __BTGetJoystickReportID() {
    int i = 1;
    if (__BTInstallKeyboard) {
        i += 2;
    }
    if (__BTInstallMouse) {
        i++;
    }
    return i;
}

int __BLEGetKeyboardReportID() {
    return 1;
}

int __BLEGetMouseReportID() {
    return __BLEInstallKeyboard ? 3 : 1;
}

int __BLEGetFeatureReportID() {
    int feature = 1;
    if (__BLEInstallKeyboard) {
        feature += 2;
    }
    if (__BLEInstallMouse) {
        feature ++;
    }
    if (__BLEInstallJoystick) {
        feature ++;
    }
    return feature;
}

int __BLEGetJoystickReportID() {
    int i = 1;
    if (__BLEInstallKeyboard) {
        i += 2;
    }
    if (__BLEInstallMouse) {
        i++;
    }
    return i;
}
