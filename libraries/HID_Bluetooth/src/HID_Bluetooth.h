#ifdef ENABLE_CLASSIC
#include "PicoBluetoothHID.h"
#endif

#ifdef ENABLE_BLE
#include "PicoBluetoothBLEHID.h"
#endif

#pragma once

//override weak declarations to include HID report to report map.
//done in each library (KeyboardBT,...)
extern void __BTInstallKeyboard() __attribute__((weak));
extern void __BTInstallJoystick() __attribute__((weak));
extern void __BTInstallMouse() __attribute__((weak));

//override weak declarations to include HID report to report map.
//done in each library (KeyboardBLE,...)
extern void __BLEInstallKeyboard() __attribute__((weak));
extern void __BLEInstallJoystick() __attribute__((weak));
extern void __BLEInstallMouse() __attribute__((weak));

//setup the report map.
//more generic function to be used with BLE & BT Classis
void __SetupHIDreportmap(void (*WeakMouse)(), void (*WeakKeyboard)(), void (*WeakJoystick)(), bool absMouse, uint16_t *report_size, uint8_t **reportmap);

//get Class of Device number for starting HID, type depends on activated libraries
uint16_t __BTGetCOD();
//get Class of Device number for starting HID, type depends on activated libraries
uint16_t __BLEGetAppearance();
int __BTGetKeyboardReportID();
int __BTGetMouseReportID();
int __BTGetJoystickReportID();

int __BLEGetKeyboardReportID();
int __BLEGetMouseReportID();
int __BLEGetJoystickReportID();
int __BLEGetFeatureReportID();
