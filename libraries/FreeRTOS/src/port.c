#ifdef PICO_RP2040
#include "../lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/port.c"
#else
#include "../lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/port.c"
#endif
