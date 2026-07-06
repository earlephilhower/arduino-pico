#ifdef __FREERTOS
#include <pico.h>

#if (PICO_SDK_VERSION_MAJOR * 100) + (PICO_SDK_VERSION_MINOR * 10) + PICO_SDK_VERSION_REVISION != 230
#error Check that this override warning is still needed
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused"
#include "../../../pico-sdk/src/rp2_common/pico_async_context/async_context_freertos.c"
#pragma GCC diagnostic pop

#endif
