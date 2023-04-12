// Simple helper header to ensure pico libs support ?BT

#ifndef ENABLE_CLASSIC
#define ENABLE_CLASSIC 0
#endif

static_assert(ENABLE_CLASSIC, "This library needs Bluetooth enabled.  Use the 'Tools->IP/Bluetooth Stack' menu in the IDE to enable it.");
