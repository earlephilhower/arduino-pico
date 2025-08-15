#ifdef __FREERTOS
#ifdef PICO_RP2350
#ifndef __riscv
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure//rp2040_config.h"
#else
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_RISC-V/include/rp2040_config.h"
#endif
#else
#include "../../../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include/rp2040_config.h"
#endif
#endif
