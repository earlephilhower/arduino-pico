#ifdef __FREERTOS
#ifdef PICO_RP2350
#ifndef __riscv
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/portmacro.h"
#else
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_RISC-V/include/portmacro.h"
#endif
#else
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include/portmacro.h"
#endif
#endif
