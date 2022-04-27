// Port.c seems to leave interrupts disabled occasionally when built w/anything other than -O0

#pragma GCC optimize ("O0")
#include "../lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/port.c"
