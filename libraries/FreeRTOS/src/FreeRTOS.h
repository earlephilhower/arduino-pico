#if !defined(__FREERTOS)
#error Please select Tools->Operating System->FreeRTOS SMP or define __FREERTOS in your platform.ini to use FreeRTOS
#else
#include "../lib/FreeRTOS-Kernel/include/FreeRTOS.h"
#endif
