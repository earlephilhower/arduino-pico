#ifdef __FREERTOS
#ifdef PICO_RP2040
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/port.c"
#else
#ifndef __riscv
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/port.c"
#else
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_RISC-V/port.c"
#endif
#endif
#endif
