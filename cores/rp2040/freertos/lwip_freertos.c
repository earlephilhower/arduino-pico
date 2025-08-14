// For FreeRTOS, we'll just use the NOSYS version because we already safely handle it
#ifdef __FREERTOS

#include "pico/async_context.h"

extern bool lwip_nosys_init(async_context_t *context);
extern void lwip_nosys_deinit(async_context_t *context);

bool lwip_freertos_init(async_context_t *context) {
    return lwip_nosys_init(context);
}
void lwip_freertos_deinit(__unused async_context_t *context) {
    lwip_nosys_deinit(context);
}
#endif
