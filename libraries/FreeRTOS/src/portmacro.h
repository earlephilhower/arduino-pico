#ifdef PICO_RP2350
#ifndef __riscv
#include "../lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/portmacro.h"
#else
#include "../lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_RISC-V/include/portmacro.h"
#endif
#else
#include "../lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include/portmacro.h"
#endif
